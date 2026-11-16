# MobileBloxV2 - Build Instructions

## Prerequisites

### Host System (MUST be x86_64)
- Linux (Ubuntu 22.04+ / Arch / Fedora) x86_64
- macOS (Intel or Apple Silicon with Rosetta) x86_64
- Windows with WSL2 (Ubuntu) x86_64
- **NOT Termux/Android on ARM64** - NDK toolchain is x86_64 only

### Required Tools
```bash
# Android NDK r26d
wget https://dl.google.com/android/repository/android-ndk-r26d-linux.zip
unzip android-ndk-r26d-linux.zip -d ~/android
export ANDROID_NDK_HOME=~/android/android-ndk-r26d
export PATH=$ANDROID_NDK_HOME:$PATH

# CMake 3.22+ (NDK includes it)
# Ninja
sudo apt install ninja-build

# Gradle 8.5+ (via wrapper)
# JDK 17+
sudo apt install openjdk-17-jdk
```

## Build Native Libraries (ARM64)

### 1. Build Dependencies
```bash
cd MobileBloxV2-main
mkdir -p build/arm64 && cd build/arm64

# Configure with CMake
cmake -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-24 \
  -DANDROID_STL=c++_shared \
  -DCMAKE_BUILD_TYPE=Release \
  ../..

# Build
ninja -j$(nproc)
```

### 2. Output
```
build/arm64/
├── libMobileBlox.so          # Main library
├── libdobby.a                # Hooking
├── libcurl.a + deps          # HTTP stack
├── libluau.a                 # Luau VM
└── ...other static libs
```

## Build Android App

### 1. Generate Gradle Wrapper
```bash
cd MobileBloxV2-main/app
gradle wrapper --gradle-version 8.5
```

### 2. Build APK
```bash
cd MobileBloxV2-main
./gradlew assembleRelease
# Output: app/build/outputs/apk/release/app-release.apk
```

## Current Build (Legacy - Visual Studio)

### Requirements
- Visual Studio 2022 17.7+
- Android NDK r21e (for Clang 5.0)
- ARM32 only

### Steps
1. Open `MobileBloxV2.sln`
2. Select `Release | ARM`
3. Build → `libMobileBlox.so` in `NativeLib/Release/ARM/`

## Environment Variables
```bash
export ANDROID_NDK_HOME=/path/to/ndk
export ANDROID_SDK_ROOT=/path/to/sdk
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk
```

## Troubleshooting

| Issue | Solution |
|-------|----------|
| `libc++_shared.so` conflict | Ensure single copy in APK, use `packagingOptions` |
| Dobby hook fails | Check SELinux, try `dobby_enable_near_branch_trampoline()` |
| Luau symbols missing | Verify `Luau::BytecodeEncoder` vtable linkage |
| JNI_OnLoad not called | Check `System.loadLibrary("MobileBlox")` name matches `TargetName` |
| Offsets wrong | Implement version detection + dynamic resolution |

## CI/CD (GitHub Actions)
```yaml
# .github/workflows/build.yml
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Setup NDK
        uses: android-actions/setup-ndk@v1
        with: { ndk-version: r26d }
      - name: Build native
        run: |
          cmake -B build/arm64 -S . -DCMAKE_TOOLCHAIN_FILE=... -DANDROID_ABI=arm64-v8a
          cmake --build build/arm64
      - name: Build APK
        run: ./gradlew assembleRelease
      - uses: actions/upload-artifact@v4
        with: { path: app/build/outputs/apk/release/*.apk }
```