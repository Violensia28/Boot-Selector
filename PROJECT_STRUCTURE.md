# 📁 BootSelector ESP32 - Project Structure

## Complete File Tree

```
BootSelector-ESP32/
├── .github/
│   └── workflows/
│       ├── build.yml           # Build & artifact generation
│       ├── release.yml         # Release automation (on tags)
│       └── pr-check.yml        # PR validation checks
│
├── src/
│   └── main.cpp               # Main BootSelector code
│
├── partitions.csv             # ESP32 partition table (4MB)
├── platformio.ini             # PlatformIO configuration
├── README.md                  # Main documentation
├── LICENSE                    # MIT License
├── CONTRIBUTING.md            # Contribution guidelines
├── .gitignore                 # Git ignore rules
└── PROJECT_STRUCTURE.md       # This file

```

## 📄 File Descriptions

### Core Files

#### `src/main.cpp`
**Purpose:** Main firmware untuk BootSelector
**Size Target:** < 512KB compiled
**Key Functions:**
- `setupSafeGPIO()` - Keamanan GPIO pre-boot
- `getPartitionInfo()` - Info partisi untuk UI
- `handleOTAUpload()` - Upload firmware ke slot spesifik
- `handleBootSpot()` / `handleBootCharger()` - Boot switching

**Dependencies:**
```cpp
#include <WiFi.h>          // SoftAP
#include <WebServer.h>     // HTTP server
#include <esp_ota_ops.h>   // OTA operations
#include <esp_partition.h> // Partition management
```

#### `partitions.csv`
**Purpose:** Custom partition table untuk ESP32 4MB
**Format:** CSV (Name, Type, SubType, Offset, Size, Flags)

**Layout:**
```
Offset     Size        Partition
0x9000     24KB        nvs
0xF000     8KB         otadata
0x11000    4KB         phy_init
0x20000    512KB       factory (BootSelector)
0xA0000    1.25MB      ota_0 (Spot Welding)
0x1E0000   1.25MB      ota_1 (DIY Charger)
0x320000   832KB       spiffs
```

**Modifikasi:** Jika aplikasi > 1.25MB, naikkan ukuran OTA slot dan kurangi SPIFFS.

#### `platformio.ini`
**Purpose:** Build configuration
**Key Settings:**
```ini
platform = espressif32
board = esp32dev
framework = arduino
board_build.partitions = partitions.csv
monitor_speed = 115200
build_flags = -D BOOTSEL_AP_SSID="..." -D BOOTSEL_AP_PASS="..."
```

**Customization:**
- Change AP credentials via `build_flags`
- Add libraries via `lib_deps`
- Adjust monitor speed

### Documentation Files

#### `README.md`
**Sections:**
1. Features & Overview
2. Partition Layout
3. Build & Flash Instructions
4. Usage Guide
5. Safety Guidelines
6. Troubleshooting
7. API Reference
8. Development Ideas

**Audience:** End users, makers, developers

#### `CONTRIBUTING.md`
**Sections:**
1. Setup instructions
2. Code style guide
3. Commit conventions
4. Testing checklist
5. PR process
6. Good first issues

**Audience:** Contributors

#### `LICENSE`
MIT License - permissive, allows commercial use

### CI/CD Workflows

#### `.github/workflows/build.yml`
**Trigger:** Push, PR, manual
**Jobs:**
1. **build** - Compile firmware, create artifacts
2. **lint** - Code quality checks

**Artifacts:**
- `bootselector.bin` - Main firmware
- `bootloader.bin` - ESP32 bootloader
- `partitions.bin` - Compiled partition table
- `flash.bat` / `flash.sh` - Flash scripts
- `build_info.txt` - Build metadata

**Outputs:**
- ZIP package with all files
- Uploadable artifacts (90 days retention)

#### `.github/workflows/release.yml`
**Trigger:** Git tags (`v*.*.*`)
**Process:**
1. Build firmware
2. Package all files + scripts
3. Generate changelog
4. Create GitHub Release
5. Upload assets

**Release Assets:**
- Complete ZIP package
- Individual BIN files
- Flash scripts
- Build info
- SHA256 checksums

**Release Notes:**
- Version info
- Changelog
- Quick start guide
- Download instructions
- Flash commands

#### `.github/workflows/pr-check.yml`
**Trigger:** Pull Requests
**Checks:**
1. **Build Test** - Compile without errors
2. **Size Check** - Firmware < 512KB
3. **Partition Validation** - All partitions present, total < 4MB
4. **Security Scan** - Check for hardcoded secrets
5. **Code Quality** - cpplint checks
6. **Documentation** - README completeness

**PR Comments:**
Automated comment with:
- Build status
- Firmware size & percentage
- Partition validation results

## 🔄 Workflow Diagram

```
┌─────────────────────────────────────────────────────────┐
│  Developer Workflow                                     │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
            ┌──────────────────────────┐
            │  Edit Code (main.cpp)    │
            └──────────────────────────┘
                          │
                          ▼
            ┌──────────────────────────┐
            │  Commit & Push           │
            └──────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│  GitHub Actions (build.yml)                             │
├─────────────────────────────────────────────────────────┤
│  1. Checkout code                                       │
│  2. Setup PlatformIO                                    │
│  3. Build firmware                                      │
│  4. Generate artifacts                                  │
│  5. Upload to GitHub                                    │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│  If Tag (v*.*.*)                                        │
├─────────────────────────────────────────────────────────┤
│  release.yml:                                           │
│  1. Create complete package                             │
│  2. Generate changelog                                  │
│  3. Create GitHub Release                               │
│  4. Upload release assets                               │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
            ┌──────────────────────────┐
            │  Users Download Release  │
            └──────────────────────────┘
                          │
                          ▼
            ┌──────────────────────────┐
            │  Flash to ESP32          │
            │  (via flash.sh/.bat)     │
            └──────────────────────────┘
```

## 🚀 Release Process

### Creating a New Release

```bash
# 1. Update version di code (jika perlu)
# 2. Commit semua perubahan
git add .
git commit -m "chore: prepare release v1.0.0"

# 3. Create & push tag
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0

# 4. GitHub Actions otomatis akan:
#    - Build firmware
#    - Package files
#    - Create release
#    - Upload assets
```

### Version Numbering (Semantic Versioning)

Format: `vMAJOR.MINOR.PATCH`

- **MAJOR**: Breaking changes (API changes, incompatible updates)
- **MINOR**: New features (backward compatible)
- **PATCH**: Bug fixes

**Examples:**
- `v1.0.0` - Initial release
- `v1.1.0` - Add MD5 verification (new feature)
- `v1.1.1` - Fix WiFi timeout bug
- `v2.0.0` - Change partition layout (breaking)

## 📦 Build Artifacts

### Dari GitHub Actions

**Build Artifacts** (setiap push):
```
bootselector-esp32-{sha}/
├── bootselector.bin    # Firmware (flash ke 0x10000)
├── bootloader.bin      # Bootloader (flash ke 0x1000)
├── partitions.bin      # Partition table (flash ke 0x8000)
├── partitions.csv      # CSV reference
├── flash.bat           # Windows flash script
├── flash.sh            # Linux/Mac flash script
└── build_info.txt      # Build metadata
```

**Release Assets** (pada tag):
```
BootSelector-v1.0.0.zip
├── [semua file di atas]
├── README.md
├── LICENSE
├── BUILD_INFO.txt
└── src/
    └── main.cpp
```

## 🔧 Customization Points

### 1. WiFi Credentials
**File:** `platformio.ini`
```ini
build_flags = 
  -D BOOTSEL_AP_SSID=\"MyCustomSSID\"
  -D BOOTSEL_AP_PASS=\"MyPassword123\"
```

### 2. Partition Sizes
**File:** `partitions.csv`
```csv
# Adjust sizes based on your needs
ota_0, app, ota_0, 0xA0000, 0x180000,  # 1.5MB instead of 1.25MB
```

### 3. GPIO Safety List
**File:** `src/main.cpp`
```cpp
const uint8_t SAFE_PINS[] = {4, 5, 13, 14, 16, 17, /* add more */};
```

### 4. UI Styling
**File:** `src/main.cpp` (HTML_PAGE)
```html
<style>
  body { background: #your-color; }
  /* Customize CSS */
</style>
```

## 🎯 Future Enhancements

### Planned Features
- [ ] Image verification (MD5/SHA256)
- [ ] GPIO-based boot selection
- [ ] Double-reset detection
- [ ] Multi-language UI
- [ ] HTTPS support
- [ ] OTA rollback mechanism
- [ ] Metrics/logging

### Architecture Improvements
- [ ] Unit tests (Google Test)
- [ ] Integration tests
- [ ] Hardware-in-loop testing
- [ ] Performance benchmarks
- [ ] Memory profiling

## 📊 Statistics

**Target Metrics:**
- Firmware size: < 400KB (target < 512KB limit)
- Build time: < 2 minutes
- Flash time: < 30 seconds
- Web UI load: < 1 second
- OTA upload: ~1MB/minute

## 🆘 Quick Links

- [Main README](README.md)
- [Contributing Guide](CONTRIBUTING.md)
- [License](LICENSE)
- [Issues](../../issues)
- [Releases](../../releases)

---

**Last Updated:** 2024-10-24  
**Maintainer:** @Violensia28
