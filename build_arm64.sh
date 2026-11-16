#!/bin/bash
# MobileBlox V2 - ARM64 Build Script for S25 FE (Android 14+)
# Run on x86_64 Linux (Ubuntu 22.04+, Debian 12+, Fedora 38+, Arch, WSL2)
# Output: MobileBloxV2-arm64.apk + libMobileBlox-arm64.so

set -euo pipefail

# =============================================================================
# CONFIGURATION
# =============================================================================
ANDROID_NDK_VERSION="r26d"
ANDROID_API_LEVEL=24          # Minimum (Android 7.0) - S25 FE runs 14+
TARGET_ABI="arm64-v8a"
BUILD_TYPE="Release"
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build/${TARGET_ABI}"
ARTIFACTS_DIR="${PROJECT_DIR}/artifacts"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log() { echo -e "${BLUE}[BUILD]${NC} $*"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
error() { echo -e "${RED}[ERROR]${NC} $*"; exit 1; }
ok() { echo -e "${GREEN}[OK]${NC} $*"; }

# =============================================================================
# CHECK HOST ARCHITECTURE
# =============================================================================
HOST_ARCH=$(uname -m)
if [[ "$HOST_ARCH" != "x86_64" ]]; then
    error "This script must run on x86_64 host. Current: $HOST_ARCH
Use Docker (Dockerfile.build) or GitHub Actions for ARM64 hosts."
fi
ok "Host architecture: $HOST_ARCH ✓"

# =============================================================================
# CHECK DEPENDENCIES
# =============================================================================
check_cmd() {
    command -v "$1" >/dev/null 2>&1 || error "Missing: $1 (install: $2)"
}

log "Checking dependencies..."
check_cmd cmake "sudo apt install cmake"
check_cmd ninja "sudo apt install ninja-build"
check_cmd java "sudo apt install openjdk-17-jdk"
check_cmd git "sudo apt install git"
check_cmd wget "sudo apt install wget"
check_cmd unzip "sudo apt install unzip"
check_cmd python3 "sudo apt install python3"
ok "All dependencies found"

# =============================================================================
# SETUP ANDROID NDK
# =============================================================================
setup_ndk() {
    log "Setting up Android NDK ${ANDROID_NDK_VERSION}..."
    
    if [[ -z "${ANDROID_NDK_HOME:-}" ]]; then
        export ANDROID_NDK_HOME="${HOME}/android-ndk-${ANDROID_NDK_VERSION}"
    fi
    
    if [[ ! -d "$ANDROID_NDK_HOME" ]]; then
        log "Downloading NDK..."
        wget -q "https://dl.google.com/android/repository/android-ndk-${ANDROID_NDK_VERSION}-linux.zip" -O /tmp/ndk.zip
        unzip -q /tmp/ndk.zip -d "${HOME}"
        rm /tmp/ndk.zip
    fi
    
    if [[ ! -f "$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" ]]; then
        error "NDK not found at $ANDROID_NDK_HOME"
    fi
    
    export PATH="$ANDROID_NDK_HOME:$PATH"
    ok "NDK ready: $ANDROID_NDK_HOME"
}

# =============================================================================
# BUILD OPENSSL/CURL/NGTCP2 STACK FOR ARM64 (using vcpkg)
# =============================================================================
setup_vcpkg() {
    log "Setting up vcpkg for ARM64 Android..."
    
    VCPKG_ROOT="${HOME}/vcpkg"
    if [[ ! -d "$VCPKG_ROOT" ]]; then
        git clone --depth 1 https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT"
        "$VCPKG_ROOT/bootstrap-vcpkg.sh"
    fi
    
    # Create Android ARM64 triplet
    TRIPLET_FILE="$VCPKG_ROOT/triplets/arm64-android.cmake"
    if [[ ! -f "$TRIPLET_FILE" ]]; then
        cat > "$TRIPLET_FILE" << 'EOF'
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_ANDROID_NDK_PATH $ENV{ANDROID_NDK_HOME})
set(VCPKG_CMAKE_SYSTEM_NAME Android)
set(VCPKG_CMAKE_SYSTEM_VERSION 24)
set(VCPKG_CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_ANDROID_NDK $ENV{ANDROID_NDK_HOME})
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION clang)
set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_VERSION 24)
set(CMAKE_ANDROID_STL_TYPE c++_shared)
EOF
    fi
    
    # Install dependencies
    log "Installing OpenSSL, curl, ngtcp2, nghttp2, nghttp3 for ARM64..."
    "$VCPKG_ROOT/vcpkg" install --triplet arm64-android \
        openssl \
        curl \
        ngtcp2 \
        nghttp2 \
        nghttp3 \
        zlib \
        --overlay-ports="${PROJECT_DIR}/vcpkg-overlays" 2>/dev/null || \
    "$VCPKG_ROOT/vcpkg" install --triplet arm64-android \
        openssl \
        curl \
        ngtcp2 \
        nghttp2 \
        nghttp3 \
        zlib
    
    export VCPKG_INSTALL_DIR="$VCPKG_ROOT/installed/arm64-android"
    ok "vcpkg dependencies installed at $VCPKG_INSTALL_DIR"
}

# =============================================================================
# CONFIGURE AND BUILD NATIVE LIBRARY
# =============================================================================
build_native() {
    log "Configuring CMake for $TARGET_ABI..."
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    cmake -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
        -DCMAKE_PREFIX_PATH="$VCPKG_INSTALL_DIR" \
        -DANDROID_ABI="$TARGET_ABI" \
        -DANDROID_PLATFORM="android-$ANDROID_API_LEVEL" \
        -DANDROID_STL=c++_shared \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DOpenSSL_ROOT_DIR="$VCPKG_INSTALL_DIR" \
        -DCURL_ROOT_DIR="$VCPKG_INSTALL_DIR" \
        "$PROJECT_DIR/../.."
    
    log "Building native library..."
    ninja -j$(nproc)
    
    # Verify output
    if [[ -f "libMobileBlox.so" ]]; then
        ok "libMobileBlox.so built successfully"
        file libMobileBlox.so
    else
        error "libMobileBlox.so not found"
    fi
}

# =============================================================================
# BUILD APK
# =============================================================================
build_apk() {
    log "Building APK..."
    cd "$PROJECT_DIR"
    
    # Ensure gradlew executable
    chmod +x gradlew
    
    # Build release APK
    ./gradlew assembleRelease --no-daemon \
        -Dorg.gradle.jvmargs="-Xmx4g -XX:MaxMetaspaceSize=1g" \
        -Pandroid.ndkPath="$ANDROID_NDK_HOME"
    
    # Verify APK
    APK_PATH="app/build/outputs/apk/release/app-release.apk"
    if [[ -f "$APK_PATH" ]]; then
        ok "APK built: $APK_PATH"
        
        # Verify ARM64 libs in APK
        unzip -l "$APK_PATH" | grep "libMobileBlox.so" | grep arm64-v8a || \
            warn "ARM64 lib not found in APK - check packagingOptions"
    else
        error "APK not found at $APK_PATH"
    fi
}

# =============================================================================
# PACKAGE ARTIFACTS
# =============================================================================
package_artifacts() {
    log "Packaging artifacts..."
    
    mkdir -p "$ARTIFACTS_DIR"
    
    # Copy APK
    cp "app/build/outputs/apk/release/app-release.apk" \
        "$ARTIFACTS_DIR/MobileBloxV2-arm64-$(date +%Y%m%d).apk"
    
    # Copy native lib
    cp "$BUILD_DIR/libMobileBlox.so" \
        "$ARTIFACTS_DIR/libMobileBlox-arm64.so"
    
    # Generate info file
    cat > "$ARTIFACTS_DIR/BUILD_INFO.txt" << EOF
MobileBlox V2 - ARM64 Build
===========================
Date: $(date)
Host: $(uname -a)
NDK: $ANDROID_NDK_VERSION
API Level: $ANDROID_API_LEVEL
ABI: $TARGET_ABI
Build Type: $BUILD_TYPE
Target Device: Samsung Galaxy S25 FE (Android 14+)
Git Commit: $(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
EOF
    
    ok "Artifacts in $ARTIFACTS_DIR:"
    ls -lh "$ARTIFACTS_DIR"
}

# =============================================================================
# INSTALL TO DEVICE (optional)
# =============================================================================
install_to_device() {
    if command -v adb >/dev/null 2>&1; then
        log "Installing to connected device..."
        adb install -r "$ARTIFACTS_DIR/MobileBloxV2-arm64-$(date +%Y%m%d).apk" && \
            ok "Installed successfully" || \
            warn "Install failed - device may not be connected"
    else
        warn "adb not found - skip device install"
    fi
}

# =============================================================================
# MAIN
# =============================================================================
main() {
    log "=========================================="
    log "MobileBlox V2 - ARM64 Build for S25 FE"
    log "=========================================="
    
    setup_ndk
    setup_vcpkg
    build_native
    build_apk
    package_artifacts
    
    log "=========================================="
    log "BUILD COMPLETE"
    log "=========================================="
    ok "APK: $ARTIFACTS_DIR/MobileBloxV2-arm64-$(date +%Y%m%d).apk"
    ok "Lib: $ARTIFACTS_DIR/libMobileBlox-arm64.so"
    
    # Offer to install
    if [[ "${1:-}" == "--install" ]]; then
        install_to_device
    fi
}

main "$@"