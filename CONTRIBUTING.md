# Contributing to BootSelector ESP32

Terima kasih atas minat Anda untuk berkontribusi! 🎉

## 🚀 Quick Start untuk Kontributor

### Prerequisites

- Python 3.8+
- PlatformIO Core
- Git
- ESP32 development board (untuk testing)

### Setup Development Environment

```bash
# Clone repository
git clone https://github.com/yourusername/BootSelector-ESP32.git
cd BootSelector-ESP32

# Install PlatformIO
pip install platformio

# Build project
pio run

# Upload ke ESP32
pio run -t upload
```

## 📝 Contribution Guidelines

### 1. Membuat Issue

Sebelum membuat PR, buat issue terlebih dahulu untuk:
- Bug reports
- Feature requests
- Documentation improvements

**Template Bug Report:**
```markdown
**Deskripsi Bug:**
[Jelaskan bug secara singkat]

**Langkah Reproduksi:**
1. Flash firmware versi X
2. Upload file Y
3. Klik tombol Z
4. Lihat error

**Expected Behavior:**
[Apa yang seharusnya terjadi]

**Actual Behavior:**
[Apa yang benar-benar terjadi]

**Environment:**
- Board: ESP32-WROOM-32
- Flash Size: 4MB
- PlatformIO Version: X.X.X
- Serial Output: [paste log]
```

### 2. Forking & Branching

```bash
# Fork repository di GitHub, lalu:
git clone https://github.com/YOUR_USERNAME/BootSelector-ESP32.git
cd BootSelector-ESP32

# Buat branch baru
git checkout -b feature/nama-fitur
# atau
git checkout -b fix/nama-bug
```

**Branch Naming Convention:**
- `feature/` - Fitur baru
- `fix/` - Bug fix
- `docs/` - Dokumentasi
- `refactor/` - Code refactoring
- `test/` - Testing improvements

### 3. Code Style

#### C++ Style Guide

```cpp
// ✅ GOOD: Descriptive names, proper formatting
void handleOTAUpload(esp_partition_subtype_t target, const char* name) {
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("[OTA] Starting upload: %s\n", name);
    // ...
  }
}

// ❌ BAD: Poor naming, no comments
void h(int t,char*n){HTTPUpload&u=server.upload();if(u.status==0){Serial.printf("%s\n",n);}}
```

**Aturan:**
- Indentasi: 2 spaces (bukan tabs)
- Line length: maksimal 100 karakter
- Naming: `camelCase` untuk fungsi, `UPPER_CASE` untuk constants
- Selalu tambahkan comments untuk fungsi kompleks
- Include guards: `#ifndef HEADER_NAME_H`

#### Commit Messages

Format: `type(scope): subject`

**Types:**
- `feat`: Fitur baru
- `fix`: Bug fix
- `docs`: Dokumentasi
- `style`: Formatting
- `refactor`: Code restructuring
- `test`: Testing
- `chore`: Maintenance

**Examples:**
```bash
git commit -m "feat(ota): add MD5 verification before flash"
git commit -m "fix(wifi): resolve AP connection timeout"
git commit -m "docs(readme): add troubleshooting for upload errors"
```

### 4. Testing

Sebelum submit PR, test minimal:

#### Manual Testing Checklist

- [ ] Build berhasil tanpa warning
- [ ] Flash ke ESP32 berhasil
- [ ] AP WiFi muncul dengan benar
- [ ] Web UI bisa diakses
- [ ] Upload firmware ke ota_0 berhasil
- [ ] Upload firmware ke ota_1 berhasil
- [ ] Switch ke ota_0 dan boot aplikasi berhasil
- [ ] Switch ke ota_1 dan boot aplikasi berhasil
- [ ] GPIO safety bekerja (output tidak aktif)
- [ ] Serial monitor output informatif

#### Test dengan Berbagai Kondisi

```bash
# Test build untuk berbagai environment
pio run -e esp32dev

# Check code quality
pip install cpplint
cpplint --recursive src/

# Test firmware size
ls -lh .pio/build/esp32dev/firmware.bin
# Harus < 512KB
```

### 5. Pull Request Process

#### Sebelum Submit PR

1. **Update dari upstream:**
   ```bash
   git remote add upstream https://github.com/ORIGINAL_OWNER/BootSelector-ESP32.git
   git fetch upstream
   git rebase upstream/main
   ```

2. **Run checks:**
   ```bash
   pio run
   # Test di hardware jika memungkinkan
   ```

3. **Update documentation:**
   - Update README.md jika ada perubahan API/usage
   - Update CHANGELOG (buat entry di "Unreleased")
   - Tambahkan comments di code

#### Submit PR

**PR Title Format:**
```
[Type] Brief description (max 72 chars)
```

**PR Description Template:**
```markdown
## Deskripsi
[Jelaskan perubahan yang dibuat]

## Motivasi
[Kenapa perubahan ini diperlukan?]

## Tipe Perubahan
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Build successful
- [ ] Tested on hardware
- [ ] Manual testing passed
- [ ] Documentation updated

## Screenshots (jika applicable)
[Tambahkan screenshot untuk perubahan UI]

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex code
- [ ] Documentation updated
- [ ] No new warnings generated
- [ ] Firmware size within limits (<512KB)

## Related Issues
Closes #123
```

#### Review Process

1. **Automated checks** akan berjalan (GitHub Actions)
2. **Maintainer review** - bisa minta perubahan
3. Setelah approved → **merge to main**

### 6. Specific Areas untuk Kontribusi

#### 🐛 Bug Fixes (Priority: High)

- Upload timeout issues
- WiFi stability
- OTA verification errors
- GPIO safety edge cases

#### ✨ Features (Priority: Medium)

- MD5 verification before boot
- GPIO boot selection (hold button)
- Double-reset detection
- Web UI improvements (progress, animations)
- Multi-language support
- Backup/restore configuration

#### 📚 Documentation (Priority: High)

- Translation (English, Indonesian)
- Video tutorials
- Wiring diagrams
- Common pitfalls guide

#### 🧪 Testing (Priority: Medium)

- Unit tests
- Integration tests
- Hardware compatibility testing
- Performance benchmarks

## 🎯 Good First Issues

Label `good-first-issue` untuk newcomer-friendly tasks:

- Documentation typos
- Code comments improvement
- Simple UI enhancements
- Testing on different ESP32 variants

## 💬 Communication

- **GitHub Issues** - Bug reports, features
- **GitHub Discussions** - Q&A, ideas
- **Pull Requests** - Code contributions

## 📜 Code of Conduct

- Be respectful and inclusive
- Constructive criticism only
- Help others learn
- No harassment tolerated

## 🏆 Recognition

Contributors akan di-list di:
- README.md (Contributors section)
- Release notes
- Project credits

## ❓ Questions?

Tidak yakin dari mana mulai? Buat discussion di GitHub atau comment di issue yang ada.

Terima kasih sudah berkontribusi! 🙏
