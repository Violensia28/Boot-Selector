![Build](https://github.com/Violensia28/Boot-Selector/workflows/Build%20BootSelector%20ESP32/badge.svg)
![Release](https://github.com/Violensia28/Boot-Selector/workflows/Release%20BootSelector/badge.svg)
![License](https://img.shields.io/github/v/release/Violensia28/Boot-Selector)
![Release](https://img.shields.io/github/v/release/Violensia28/Boot-Selector)

# BootSelector ESP32

**Dual-boot manager untuk ESP32 (4MB Flash)** - Mengelola dua aplikasi independen: **Spot Welding** dan **DIY Charger** dalam satu perangkat.

## 🎯 Fitur Utama

- ✅ **Web UI** untuk upload firmware via browser (tanpa kabel)
- ✅ **Dual OTA slots** - `ota_0` untuk Spot, `ota_1` untuk Charger
- ✅ **Boot selector** - Switch antar aplikasi tanpa re-flash
- ✅ **Safety first** - GPIO pre-boot dalam keadaan aman
- ✅ **Recovery mode** - Selalu bisa kembali ke BootSelector

## 📊 Layout Partisi (4MB Flash)

```
Partition         Address    Size        Purpose
─────────────────────────────────────────────────────
nvs               0x9000     24KB        Non-volatile storage
otadata           0xF000     8KB         OTA boot info
phy_init          0x11000    4KB         PHY calibration
factory           0x20000    512KB       BootSelector (ini!)
ota_0             0xA0000    1.25MB      Spot Welding app
ota_1             0x1E0000   1.25MB      DIY Charger app
spiffs            0x320000   832KB       File system
─────────────────────────────────────────────────────
TOTAL: 4MB (0x400000)
```

> ⚠️ **Penting:** Ukuran maksimal setiap aplikasi adalah **~1.25MB**. Jika aplikasi Anda lebih besar, edit `partitions.csv` dan naikkan slot OTA menjadi `0x160000` (1.375MB), kemudian kurangi ukuran SPIFFS.

## 🚀 Quick Start

### 1. Build & Flash BootSelector (Pertama Kali)

**Dengan PlatformIO:**
```bash
# Clone atau extract repo
cd BootSelector-ESP32

# Build dan upload
pio run -t upload

# Monitor serial (opsional)
pio device monitor
```

**Dengan Arduino IDE:**
1. Buka `src/main.cpp` dalam Arduino IDE
2. Pilih board: **ESP32 Dev Module**
3. Tools → Partition Scheme → **Custom** → Pilih file `partitions.csv`
4. Upload

### 2. Koneksi ke BootSelector

Setelah flash berhasil:

1. **Cari WiFi AP** bernama `BootSelector`
2. **Password:** `12345678`
3. Buka browser: **http://192.168.4.1**

### 3. Upload Aplikasi

Di Web UI:

1. **Upload Spot Welding:**
   - Pilih file `spotwelding.bin`
   - Klik **Upload ke OTA_0**
   - Tunggu hingga selesai (progress bar)

2. **Upload DIY Charger:**
   - Pilih file `diycharger.bin`
   - Klik **Upload ke OTA_1**
   - Tunggu hingga selesai

### 4. Switch Antar Aplikasi

- **Boot Spot Welding:** Klik tombol **Boot Spot Welding**
- **Boot DIY Charger:** Klik tombol **Boot DIY Charger**

Perangkat akan reboot dan masuk ke aplikasi yang dipilih.

### 5. Kembali ke BootSelector

Jika aplikasi crash atau ingin switch mode:

1. **Putuskan power** ESP32
2. **Flash ulang** BootSelector: `pio run -t upload`
3. Atau gunakan **GPIO boot button** (jika diimplementasikan)

## 🔒 Keamanan GPIO

BootSelector mengamankan GPIO berikut saat boot:
```cpp
GPIO: 4, 5, 13, 14, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
Mode: INPUT_PULLDOWN (tidak aktif, aman untuk SSR/MOSFET)
```

**Strapping pins** (0, 2, 12, 15) dibiarkan default untuk menghindari boot issues.

### Rekomendasi untuk Aplikasi

Di `setup()` setiap aplikasi (Spot/Charger), **inisialisasi pin kontrol ke LOW** sebelum enable:

```cpp
void setup() {
  // Safety first - set control pins LOW
  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, LOW);  // ✅ SSR OFF
  
  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(MOSFET_PIN, LOW);  // ✅ MOSFET OFF
  
  // ... init sensors, baru enable output
}
```

## 🔄 Kompatibilitas dengan Aplikasi Existing

### Build Aplikasi dengan Partition Table yang Sama

Kedua aplikasi (Spot & Charger) **HARUS** dibangun dengan `partitions.csv` yang sama:

**PlatformIO:**
```ini
[env:myapp]
platform = espressif32
board = esp32dev
framework = arduino
board_build.partitions = partitions.csv  ; ← PENTING!
```

**Arduino IDE:**
- Tools → Partition Scheme → Custom → Pilih `partitions.csv`

### Pisahkan Data Antar Aplikasi

**NVS (Preferences):**
```cpp
// Di Spot Welding
Preferences prefs;
prefs.begin("nvs_spot", false);  // ← namespace unik

// Di DIY Charger
Preferences prefs;
prefs.begin("nvs_charger", false);  // ← namespace berbeda
```

**SPIFFS:**
```cpp
// Gunakan folder terpisah atau prefix nama file
SPIFFS.begin(false);  // false = jangan auto-format!

// Spot: /spot_config.json
// Charger: /charger_config.json
```

> ⚠️ **Hindari** `SPIFFS.begin(true)` yang akan format seluruh filesystem!

### OTA dari Dalam Aplikasi

Aplikasi tetap bisa melakukan OTA sendiri (update firmware via `/update` endpoint):

- **Spot Welding OTA** → akan menulis ke `ota_1` (slot "lain")
- **DIY Charger OTA** → akan menulis ke `ota_0` (slot "lain")

Setelah OTA berhasil, aplikasi akan reboot ke firmware baru di slot yang sama.

## 🛠️ Troubleshooting

### Upload Gagal

**Cek ukuran file:**
```bash
ls -lh spotwelding.bin
# Harus < 1.25MB (1310720 bytes)
```

**Jika file terlalu besar:**
1. Edit `partitions.csv`:
   ```csv
   ota_0,  app,  ota_0,  0xA0000,  0x160000,  # 1.375MB
   ota_1,  app,  ota_1,  0x200000, 0x160000,  # 1.375MB
   spiffs, data, spiffs, 0x360000, 0x90000,   # 576KB
   ```
2. Rebuild BootSelector
3. **Flash ulang semua** (BootSelector + kedua aplikasi)

### Partisi Tidak Ditemukan

**Gejala:** `/info` menampilkan `OTA_0: Not found`

**Solusi:**
- Pastikan `platformio.ini` atau Arduino IDE menggunakan `partitions.csv`
- Erase flash: `pio run -t erase` → flash ulang

### Aplikasi Crash Setelah Upload

**Kemungkinan penyebab:**
1. **File corrupt** saat upload → upload ulang
2. **Partition mismatch** → rebuild aplikasi dengan `partitions.csv` yang sama
3. **SPIFFS conflict** → pastikan tidak ada auto-format

**Recovery:**
1. Kembali ke BootSelector (flash ulang)
2. Upload ulang firmware yang benar

### Web UI Tidak Bisa Diakses

**Cek koneksi:**
```bash
# Ping dari komputer
ping 192.168.4.1
```

**Cek serial monitor:**
```
[WiFi] AP IP: 192.168.4.1
[HTTP] Server started on port 80
```

Jika tidak ada output → flash ulang BootSelector.

## 📈 Pengembangan Lanjutan (Opsional)

### 1. Verifikasi Image Sebelum Boot

Tambahkan validasi di `handleBootSpot()` / `handleBootCharger()`:

```cpp
#include <esp_image_format.h>

esp_err_t err = esp_image_verify(
  ESP_IMAGE_VERIFY, 
  &ota0->address, 
  NULL
);
if (err != ESP_OK) {
  server.send(500, "text/plain", "Invalid firmware image!");
  return;
}
```

### 2. Boot Selection via GPIO

Tambahkan di `setup()`:

```cpp
#define BOOT_BUTTON_PIN 0  // GPIO0 (BOOT button)

pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
delay(100);

if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
  // Tahan BOOT button saat power-on
  // Tampilkan menu atau boot ke slot spesifik
  Serial.println("Boot button pressed!");
}
```

### 3. Double-Reset Detection

Gunakan library [ESP_DoubleResetDetector](https://github.com/khoih-prog/ESP_DoubleResetDetector) untuk revert ke factory saat double-reset.

### 4. Watchdog & Auto-Rollback

```cpp
#include <esp_task_wdt.h>

// Di aplikasi: jika crash berulang, rollback ke slot lain
if (boot_count > 3) {
  esp_ota_set_boot_partition(other_partition);
  ESP.restart();
}
```

### 5. Password Protection

Tambahkan autentikasi di WebServer:

```cpp
server.on("/", [](){
  if (!server.authenticate("admin", "password")) {
    return server.requestAuthentication();
  }
  handleRoot();
});
```

## 📋 Acceptance Tests Checklist

Sebelum deployment, pastikan semua test ini **PASS**:

### ✅ Test 1: Flash Pertama
- [ ] Board boot dan AP `BootSelector` muncul
- [ ] Koneksi ke AP berhasil
- [ ] Web UI bisa diakses di `http://192.168.4.1`
- [ ] `/info` menampilkan semua partisi dengan alamat & ukuran yang benar

### ✅ Test 2: Upload Firmware
- [ ] Upload `spotwelding.bin` → Progress 100% → "Upload OK"
- [ ] Upload `diycharger.bin` → Progress 100% → "Upload OK"
- [ ] Upload file > 1.25MB → Error (ditolak)
- [ ] Upload file non-`.bin` → Error (ditolak)

### ✅ Test 3: Boot Switching
- [ ] Klik **Boot Spot** → Reboot → Aplikasi Spot berjalan
- [ ] Power cycle → Flash BootSelector → Klik **Boot Charger** → Aplikasi Charger berjalan
- [ ] Alternatif antar aplikasi bekerja tanpa issue

### ✅ Test 4: Keamanan GPIO
- [ ] Saat di BootSelector: semua output pin tidak aktif (verifikasi dengan multimeter/LED)
- [ ] Saat di aplikasi: pin kontrol berfungsi normal
- [ ] Tidak ada aktivasi SSR/MOSFET yang tidak diinginkan saat boot

### ✅ Test 5: Robustness
- [ ] Upload file corrupt → Aplikasi tidak crash, bisa upload ulang
- [ ] Disconnect WiFi saat upload → Server tetap stabil
- [ ] Power loss saat OTA → Recovery dengan flash BootSelector ulang

## 🌐 API Reference

### Endpoints

| Method | Path | Deskripsi | Response |
|--------|------|-----------|----------|
| GET | `/` | Web UI utama | HTML page |
| GET | `/info` | Info partisi | Plain text |
| GET | `/spot` | Boot ke ota_0 (Spot) | HTML → Reboot |
| GET | `/charger` | Boot ke ota_1 (Charger) | HTML → Reboot |
| POST | `/upload/spot` | Upload `.bin` ke ota_0 | "Upload OK" / Error |
| POST | `/upload/charger` | Upload `.bin` ke ota_1 | "Upload OK" / Error |

### `/info` Response Example

```
Factory: 0x20000 (524288 bytes)
OTA_0 (Spot): 0xa0000 (1310720 bytes)
OTA_1 (Charger): 0x1e0000 (1310720 bytes)
SPIFFS: 0x320000 (851968 bytes)

Active Partition: factory (0x20000)
```

### Upload Request

**Content-Type:** `multipart/form-data`

**Form field:** `file`

**Example (curl):**
```bash
curl -X POST -F "file=@spotwelding.bin" http://192.168.4.1/upload/spot
```

## 🔧 Konfigurasi Build Flags

Edit di `platformio.ini`:

```ini
build_flags = 
  -D BOOTSEL_AP_SSID=\"MyBootLoader\"   ; Ganti SSID AP
  -D BOOTSEL_AP_PASS=\"SecurePass123\"  ; Ganti password AP
```

Atau di Arduino IDE via `#define` di `main.cpp`.

## 📂 Struktur File Project

```
BootSelector-ESP32/
├── partitions.csv           # Partition table (WAJIB!)
├── platformio.ini           # PlatformIO config
├── src/
│   └── main.cpp             # Kode BootSelector
├── README.md                # Dokumentasi ini
└── LICENSE                  # MIT License
```

## 🤝 Contributing

Kontribusi sangat diterima! Silakan:

1. Fork repository ini
2. Buat branch feature (`git checkout -b feature/AmazingFeature`)
3. Commit perubahan (`git commit -m 'Add some AmazingFeature'`)
4. Push ke branch (`git push origin feature/AmazingFeature`)
5. Buat Pull Request

## 📝 Changelog

### v1.0.0 (2024-10-23)
- ✅ Initial release
- ✅ Web UI dengan upload multipart
- ✅ Dual OTA slots (ota_0 & ota_1)
- ✅ Boot selector via HTTP endpoints
- ✅ GPIO safety pre-boot
- ✅ Partition info display

## 🐛 Known Issues

1. **Upload timeout pada koneksi lambat** - Gunakan WiFi AP yang stabil
2. **SPIFFS tidak auto-mount** - Aplikasi harus handle `SPIFFS.begin(false)`
3. **No rollback mechanism** - Manual recovery via flash BootSelector

## 📖 Referensi

- [ESP32 Partition Tables](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/partition-tables.html)
- [ESP32 OTA Updates](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/ota.html)
- [PlatformIO ESP32](https://docs.platformio.org/en/latest/platforms/espressif32.html)

## 📧 Support

Jika menemukan bug atau ada pertanyaan:

1. Cek [Troubleshooting](#troubleshooting) section
2. Buka [GitHub Issues](https://github.com/yourusername/BootSelector-ESP32/issues)
3. Sertakan:
   - Serial monitor output
   - Partition table yang digunakan
   - Langkah reproduksi masalah

## ⚖️ License

MIT License - lihat file [LICENSE](LICENSE) untuk detail lengkap.

---

**Dibuat dengan ❤️ untuk komunitas ESP32 Indonesia**

**Arif's Spot Welding & DIY Charger Project** 🔧⚡🔋
