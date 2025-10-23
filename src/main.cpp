#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>

// Konfigurasi AP
#ifndef BOOTSEL_AP_SSID
#define BOOTSEL_AP_SSID "BootSelector"
#endif
#ifndef BOOTSEL_AP_PASS
#define BOOTSEL_AP_PASS "12345678"
#endif

WebServer server(80);

// GPIO Safety - daftar pin yang perlu diamankan
const uint8_t SAFE_PINS[] = {4, 5, 13, 14, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};
const int NUM_SAFE_PINS = sizeof(SAFE_PINS) / sizeof(SAFE_PINS[0]);

// Handle OTA global
esp_ota_handle_t ota_handle = 0;
const esp_partition_t* update_partition = NULL;
bool ota_in_progress = false;

// HTML UI (inline untuk kesederhanaan)
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>BootSelector ESP32</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:Arial,sans-serif;background:#1a1a2e;color:#eee;padding:20px}
.container{max-width:800px;margin:0 auto}
h1{text-align:center;margin-bottom:30px;color:#16c79a}
.card{background:#16213e;border-radius:8px;padding:20px;margin-bottom:20px;box-shadow:0 4px 6px rgba(0,0,0,0.3)}
.card h2{color:#16c79a;margin-bottom:15px;font-size:1.2em}
pre{background:#0f3460;padding:15px;border-radius:5px;overflow-x:auto;font-size:0.9em;line-height:1.5}
.upload-form{margin:15px 0}
input[type="file"]{display:block;width:100%;padding:10px;margin:10px 0;background:#0f3460;border:1px solid #16c79a;border-radius:5px;color:#eee;cursor:pointer}
input[type="file"]:hover{background:#16c79a;color:#1a1a2e}
button,a.button{display:inline-block;padding:12px 24px;margin:5px;background:#16c79a;color:#1a1a2e;border:none;border-radius:5px;cursor:pointer;text-decoration:none;font-weight:bold;transition:0.3s}
button:hover,a.button:hover{background:#0f9c77;transform:translateY(-2px)}
.boot-btn{background:#e94560}
.boot-btn:hover{background:#d63447}
.progress{display:none;margin-top:10px;height:24px;background:#0f3460;border-radius:5px;overflow:hidden}
.progress-bar{height:100%;background:#16c79a;transition:width 0.3s;display:flex;align-items:center;justify-content:center;color:#1a1a2e;font-weight:bold}
.status{margin-top:10px;padding:10px;border-radius:5px;display:none}
.status.success{background:#16c79a;color:#1a1a2e}
.status.error{background:#e94560;color:#fff}
</style>
</head>
<body>
<div class="container">
<h1>ðŸ”§ BootSelector ESP32</h1>

<div class="card">
<h2>ðŸ“Š Info Partisi</h2>
<pre id="info">Loading...</pre>
</div>

<div class="card">
<h2>âš¡ Upload Spot Welding (ota_0)</h2>
<form class="upload-form" id="form-spot">
<input type="file" id="file-spot" accept=".bin" required>
<button type="submit">Upload ke OTA_0</button>
<div class="progress" id="prog-spot"><div class="progress-bar" id="bar-spot">0%</div></div>
<div class="status" id="status-spot"></div>
</form>
</div>

<div class="card">
<h2>ðŸ”‹ Upload DIY Charger (ota_1)</h2>
<form class="upload-form" id="form-charger">
<input type="file" id="file-charger" accept=".bin" required>
<button type="submit">Upload ke OTA_1</button>
<div class="progress" id="prog-charger"><div class="progress-bar" id="bar-charger">0%</div></div>
<div class="status" id="status-charger"></div>
</form>
</div>

<div class="card">
<h2>ðŸš€ Boot Selector</h2>
<a href="/spot" class="button boot-btn" onclick="return confirm('Boot ke Spot Welding?')">Boot Spot Welding</a>
<a href="/charger" class="button boot-btn" onclick="return confirm('Boot ke DIY Charger?')">Boot DIY Charger</a>
</div>
</div>

<script>
// Load partition info
fetch('/info').then(r=>r.text()).then(t=>document.getElementById('info').textContent=t);

// Upload handler
function setupUpload(formId,fileId,endpoint,progId,barId,statusId){
const form=document.getElementById(formId);
form.onsubmit=async(e)=>{
e.preventDefault();
const file=document.getElementById(fileId).files[0];
if(!file)return;
const prog=document.getElementById(progId);
const bar=document.getElementById(barId);
const status=document.getElementById(statusId);
prog.style.display='block';
status.style.display='none';
const xhr=new XMLHttpRequest();
xhr.upload.onprogress=(e)=>{
if(e.lengthComputable){
const pct=Math.round(e.loaded/e.total*100);
bar.style.width=pct+'%';
bar.textContent=pct+'%';
}
};
xhr.onload=()=>{
prog.style.display='none';
status.style.display='block';
if(xhr.status===200){
status.className='status success';
status.textContent='âœ… Upload berhasil!';
setTimeout(()=>location.reload(),2000);
}else{
status.className='status error';
status.textContent='âŒ Upload gagal: '+xhr.responseText;
}
};
xhr.onerror=()=>{
prog.style.display='none';
status.style.display='block';
status.className='status error';
status.textContent='âŒ Koneksi error';
};
xhr.open('POST',endpoint);
const fd=new FormData();
fd.append('file',file);
xhr.send(fd);
};
}
setupUpload('form-spot','file-spot','/upload/spot','prog-spot','bar-spot','status-spot');
setupUpload('form-charger','file-charger','/upload/charger','prog-charger','bar-charger','status-charger');
</script>
</body>
</html>
)rawliteral";

// Fungsi safety: set semua GPIO ke input pulldown
void setupSafeGPIO() {
  Serial.println("[SAFETY] Setting GPIO to INPUT_PULLDOWN...");
  for (int i = 0; i < NUM_SAFE_PINS; i++) {
    pinMode(SAFE_PINS[i], INPUT_PULLDOWN);
  }
  // Strapping pins (0,2,12,15) tidak di-set output, biarkan default
  Serial.println("[SAFETY] GPIO secured.");
}

// Get partition info sebagai string
String getPartitionInfo() {
  String info = "";

  const esp_partition_t* factory = esp_partition_find_first(
    ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
  const esp_partition_t* ota0 = esp_partition_find_first(
    ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
  const esp_partition_t* ota1 = esp_partition_find_first(
    ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
  const esp_partition_t* spiffs = esp_partition_find_first(
    ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
  const esp_partition_t* running = esp_ota_get_running_partition();

  if (factory) {
    info += "Factory: 0x" + String(factory->address, HEX) +
            " (" + String(factory->size) + " bytes)\n";
  }
  if (ota0) {
    info += "OTA_0 (Spot): 0x" + String(ota0->address, HEX) +
            " (" + String(ota0->size) + " bytes)\n";
  }
  if (ota1) {
    info += "OTA_1 (Charger): 0x" + String(ota1->address, HEX) +
            " (" + String(ota1->size) + " bytes)\n";
  }
  if (spiffs) {
    info += "SPIFFS: 0x" + String(spiffs->address, HEX) +
            " (" + String(spiffs->size) + " bytes)\n";
  }
  if (running) {
    info += "\nActive Partition: ";
    if (running->subtype == ESP_PARTITION_SUBTYPE_APP_FACTORY) {
      info += "factory";
    } else if (running->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_0) {
      info += "ota_0";
    } else if (running->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_1) {
      info += "ota_1";
    } else {
      info += "unknown";
    }
    info += " (0x" + String(running->address, HEX) + ")";
  }

  return info;
}

// Handler: root page
void handleRoot() {
  server.send(200, "text/html", HTML_PAGE);
}

// Handler: info endpoint
void handleInfo() {
  server.send(200, "text/plain", getPartitionInfo());
}

// Handler: boot ke ota_0 (Spot)
void handleBootSpot() {
  Serial.println("[BOOT] Switching to ota_0 (Spot Welding)...");
  const esp_partition_t* ota0 = esp_partition_find_first(
    ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);

  if (ota0 == NULL) {
    server.send(404, "text/plain", "OTA_0 partition not found");
    return;
  }

  esp_err_t err = esp_ota_set_boot_partition(ota0);
  if (err != ESP_OK) {
    server.send(500, "text/plain", "Failed to set boot partition: " + String(err));
    return;
  }

  server.send(200, "text/html",
    "<html><body><h2>Rebooting to Spot Welding...</h2></body></html>");
  delay(1000);
  ESP.restart();
}

// Handler: boot ke ota_1 (Charger)
void handleBootCharger() {
  Serial.println("[BOOT] Switching to ota_1 (DIY Charger)...");
  const esp_partition_t* ota1 = esp_partition_find_first(
    ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);

  if (ota1 == NULL) {
    server.send(404, "text/plain", "OTA_1 partition not found");
    return;
  }

  esp_err_t err = esp_ota_set_boot_partition(ota1);
  if (err != ESP_OK) {
    server.send(500, "text/plain", "Failed to set boot partition: " + String(err));
    return;
  }

  server.send(200, "text/html",
    "<html><body><h2>Rebooting to DIY Charger...</h2></body></html>");
  delay(1000);
  ESP.restart();
}

// Handler: upload OTA ke slot spesifik
void handleOTAUpload(esp_partition_subtype_t target_subtype, const char* name) {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("[OTA] Upload Start: %s to %s\n", upload.filename.c_str(), name);

    // Cari partisi target
    update_partition = esp_partition_find_first(
      ESP_PARTITION_TYPE_APP, target_subtype, NULL);

    if (update_partition == NULL) {
      Serial.println("[OTA] Target partition not found!");
      return;
    }

    Serial.printf("[OTA] Target: 0x%x, size: %d\n",
      update_partition->address, update_partition->size);

    // Mulai OTA
    esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK) {
      Serial.printf("[OTA] Begin failed: %d\n", err);
      update_partition = NULL;
      return;
    }
    ota_in_progress = true;

  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (update_partition == NULL || !ota_in_progress) return;

    // Tulis data
    esp_err_t err = esp_ota_write(ota_handle, upload.buf, upload.currentSize);
    if (err != ESP_OK) {
      Serial.printf("[OTA] Write failed: %d\n", err);
      esp_ota_abort(ota_handle);
      ota_in_progress = false;
      update_partition = NULL;
      return;
    }
    Serial.printf("[OTA] Written: %d bytes\n", upload.totalSize);

  } else if (upload.status == UPLOAD_FILE_END) {
    if (update_partition == NULL || !ota_in_progress) {
      server.send(500, "text/plain", "Upload failed - no active session");
      return;
    }

    // Selesaikan OTA
    esp_err_t err = esp_ota_end(ota_handle);
    if (err != ESP_OK) {
      Serial.printf("[OTA] End failed: %d\n", err);
      server.send(500, "text/plain", "OTA end failed: " + String(err));
    } else {
      Serial.printf("[OTA] Success! Total: %d bytes\n", upload.totalSize);
      server.send(200, "text/plain", "Upload OK - " + String(upload.totalSize) + " bytes");
    }

    ota_in_progress = false;
    update_partition = NULL;

  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    if (ota_in_progress) {
      esp_ota_abort(ota_handle);
      ota_in_progress = false;
    }
    update_partition = NULL;
    Serial.println("[OTA] Aborted");
  }
}

// Handler wrapper untuk upload spot
void handleUploadSpot() {
  handleOTAUpload(ESP_PARTITION_SUBTYPE_APP_OTA_0, "ota_0");
}

// Handler wrapper untuk upload charger
void handleUploadCharger() {
  handleOTAUpload(ESP_PARTITION_SUBTYPE_APP_OTA_1, "ota_1");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n\n========================================");
  Serial.println("   BootSelector ESP32 v1.0");
  Serial.println("========================================\n");

  // Safety first!
  setupSafeGPIO();

  // Print partition info
  Serial.println(getPartitionInfo());
  Serial.println();

  // Setup SoftAP
  Serial.println("[WiFi] Starting Access Point...");
  Serial.printf("SSID: %s\n", BOOTSEL_AP_SSID);
  Serial.printf("Pass: %s\n", BOOTSEL_AP_PASS);

  WiFi.softAP(BOOTSEL_AP_SSID, BOOTSEL_AP_PASS);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("[WiFi] AP IP: ");
  Serial.println(IP);

  // Setup web server
  server.on("/", handleRoot);
  server.on("/info", handleInfo);
  server.on("/spot", handleBootSpot);
  server.on("/charger", handleBootCharger);
  server.on("/upload/spot", HTTP_POST,
    []() { server.send(200); },
    handleUploadSpot);
  server.on("/upload/charger", HTTP_POST,
    []() { server.send(200); },
    handleUploadCharger);

  server.begin();
  Serial.println("[HTTP] Server started on port 80");
  Serial.println("\nReady! Connect to AP and open http://192.168.4.1\n");
}

void loop() {
  server.handleClient();
  delay(2);
}
