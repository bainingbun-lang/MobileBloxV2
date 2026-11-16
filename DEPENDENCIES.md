# MobileBloxV2 - Dependencies Analysis

## Current Dependencies (Prebuilt ARM32)

### Native Libraries (armeabi-v7a)
| Library | Version | Source | Status |
|---------|---------|--------|--------|
| libcurl | ~7.88+ | Prebuilt .a | ARM32 only |
| libcrypto (OpenSSL) | ~3.0+ | Prebuilt .a | ARM32 only |
| libssl (OpenSSL) | ~3.0+ | Prebuilt .a | ARM32 only |
| libnghttp2 | ~1.52+ | Prebuilt .a | ARM32 only |
| libnghttp3 | ~1.2+ | Prebuilt .a | ARM32 only |
| libngtcp2 | ~1.0+ | Prebuilt .a | ARM32 only |
| libngtcp2_crypto | ~1.0+ | Prebuilt .a | ARM32 only |
| libdobby | ~latest | Prebuilt .a | ARM32 only |
| libc++_shared.so | NDK r21+ | Prebuilt .so | ARM32 only |

### Header-Only / Source Dependencies
| Library | Version | Type | Notes |
|---------|---------|------|-------|
| Luau | ~2023 | Source | Modified (VM forwarders) |
| Dear ImGui | ~1.89+ | Source | Android + OpenGL3 backends |
| CPR | ~1.10+ | Source | C++ Requests wrapper |
| IXWebSocket | ~12+ | Source | WebSocket client/server |
| LZ4 | ~1.9+ | Source | Compression |
| base64 | - | Header | Simple encoding |

## Required Updates for ARM64 + Modern NDK

### 1. Rebuild All Native Dependencies for arm64-v8a
```bash
# Target: Android NDK r26+, API 24+, Clang 17+
# Architectures: arm64-v8a (primary), armeabi-v7a (secondary)
```

### 2. Dependency Versions to Use (Latest Stable)
| Library | Target Version | Build Method |
|---------|----------------|--------------|
| OpenSSL | 3.2+ | NDK build or prebuilt |
| curl | 8.7+ | With OpenSSL + ngtcp2 |
| nghttp2 | 1.61+ | Static |
| nghttp3 | 1.6+ | Static |
| ngtcp2 | 1.3+ | Static |
| Dobby | Latest (main) | CMake |
| Luau | Latest (Roblox fork) | CMake |
| ImGui | 1.90+ | Source |
| CPR | 1.11+ | CMake (FetchContent) |
| IXWebSocket | 13+ | CMake |
| LZ4 | 1.10+ | Source |
| libc++ | NDK r26 bundled | Shared |

### 3. NDK & Toolchain Requirements
- **NDK**: r26d (latest LTS) or r27 beta
- **API Level**: 24 (Android 7.0) minimum, 34 target
- **Compiler**: Clang 17+ (NDK default)
- **C++ Standard**: C++20
- **STL**: c++_shared (must match app)
- **Build System**: CMake 3.22+ + Gradle 8.5+

### 4. CMake Migration Plan
```cmake
# Top-level CMakeLists.txt
cmake_minimum_required(VERSION 3.22)
project(MobileBloxV2 LANGUAGES CXX C)

# Options
option(BUILD_ARM64 "Build for arm64-v8a" ON)
option(BUILD_ARM32 "Build for armeabi-v7a" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# FetchContent for deps
include(FetchContent)
FetchContent_Declare(luau GIT_REPOSITORY https://github.com/luau-lang/luau ...)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui ...)
# ... etc
```

### 5. Gradle Integration
```gradle
// app/build.gradle.kts
android {
    namespace = "com.mobileblox.v2"
    compileSdk = 34
    defaultConfig {
        minSdk = 24
        targetSdk = 34
        ndk {
            abiFilters.add("arm64-v8a")
            // abiFilters.add("armeabi-v7a") // optional
        }
    }
    externalNativeBuild {
        cmake { path = "CMakeLists.txt" }
    }
}
```

## Luau VM Modifications Required

The current code modifies Luau source files to forward to Roblox's internal Luau. For modernization:

1. **Keep modifications minimal** - only what's needed for Roblox interop
2. **Use Luau as submodule** - track upstream, patch via CMake
3. **Forwarder pattern**: Keep in separate `roblox_forwarders.cpp` not in Luau source
4. **Bytecode compatibility**: Ensure custom encoder works with latest Luau bytecode format

## Dobby Hooking Library

Current: Prebuilt ARM32 `.a`
Target: Build from source for ARM64
- Repo: https://github.com/jmpews/Dobby
- CMake support: Yes (recent versions)
- Alternative: Frida-gum (heavier), LiteHook (lighter)

## ImGui Backend

Current: `imgui_impl_android.cpp` + `imgui_impl_opengl3.cpp`
Target: Update for modern Android + Vulkan option
- Consider `imgui_impl_vulkan.cpp` for future renderer
- Keep OpenGL ES 3.0 backend for compatibility

## HTTP/WebSocket Stack

Current: CPR + libcurl + IXWebSocket
Issues: CPR wraps curl synchronously, IXWebSocket separate
Options:
1. Keep CPR + IXWebSocket (rebuild for ARM64)
2. Migrate to `cpp-httplib` + `ixwebsocket` (lighter)
3. Use `boost::beast` + `asio` (heavy, but modern)
4. **Recommended**: Keep CPR + IXWebSocket, rebuild with CMake

## Action Items

- [ ] Create CMakeLists.txt for all dependencies
- [ ] Build OpenSSL + curl + ngtcp2 stack for ARM64
- [ ] Build Dobby for ARM64
- [ ] Integrate Luau as CMake subproject with patches
- [ ] Create Android Gradle project
- [ ] Migrate vcxproj settings to CMake
- [ ] Test native library loading on Android 10+