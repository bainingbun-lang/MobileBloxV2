#!/bin/bash
# MobileBlox V2 - Native ARM64 Build on Termux (S25 FE)
# Compila DIRECTAMENTE en el dispositivo usando toolchain de Termux
# Requiere: Termux + Termux:API + almacenamiento interno

set -euo pipefail

# =============================================================================
# CONFIGURACIÓN
# =============================================================================
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build/termux-arm64"
ARTIFACTS_DIR="${PROJECT_DIR}/artifacts"
ANDROID_API=24  # Minimum API

# Colores
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; BLUE='\033[0;34m'; NC='\033[0m'
log() { echo -e "${BLUE}[TERMUX-BUILD]${NC} $*"; }
ok() { echo -e "${GREEN}[OK]${NC} $*"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
error() { echo -e "${RED}[ERROR]${NC} $*"; exit 1; }

# =============================================================================
# VERIFICAR ENTORNO TERMUX
# =============================================================================
check_termux() {
    log "Verificando entorno Termux..."
    
    # Verificar que estamos en Termux
    [[ -n "${TERMUX_VERSION:-}" ]] || error "No estás en Termux. Instala Termux desde F-Droid/GitHub."
    
    # Verificar arquitectura
    local arch=$(uname -m)
    [[ "$arch" == "aarch64" ]] || error "Arquitectura no soportada: $arch (necesitas ARM64/S25 FE)"
    
    # Verificar Android version
    local android_ver=$(getprop ro.build.version.sdk)
    [[ $android_ver -ge 24 ]] || warn "Android API $android_ver < 24, puede haber problemas"
    
    ok "Termux $TERMUX_VERSION en $arch (Android API $android_ver)"
}

# =============================================================================
# INSTALAR DEPENDENCIAS TERMUX
# =============================================================================
install_termux_deps() {
    log "Instalando dependencias de Termux..."
    
    # Actualizar paquetes
    pkg update -y && pkg upgrade -y
    
    # Herramientas de build
    pkg install -y \
        clang \
        lld \
        cmake \
        ninja \
        make \
        pkg-config \
        git \
        wget \
        unzip \
        openjdk-17 \
        gradle \
        python3 \
        binutils \
        file \
        patchelf
    
    # Librerías del sistema (Termux usa libc++ que incluye libc++abi)
    pkg install -y \
        libc++ \
        libandroid-support \
        libiconv \
        zlib \
        openssl \
        libcurl \
        libnghttp2
    
    ok "Dependencias instaladas"
}

# =============================================================================
# CONFIGURAR TOOLCHAIN TERMUX -> ANDROID (usa sysroot nativo de Termux)
# =============================================================================
setup_termux_toolchain() {
    log "Configurando toolchain Termux nativo para Android ARM64..."
    
    export TERMUX_PREFIX="/data/data/com.termux/files/usr"
    
    # Termux ya ES un entorno Android ARM64 - usar su sysroot nativo
    export ANDROID_SYSROOT="$TERMUX_PREFIX"
    
    export CC="$TERMUX_PREFIX/bin/clang"
    export CXX="$TERMUX_PREFIX/bin/clang++"
    export AR="$TERMUX_PREFIX/bin/llvm-ar"
    export NM="$TERMUX_PREFIX/bin/llvm-nm"
    export STRIP="$TERMUX_PREFIX/bin/llvm-strip"
    export RANLIB="$TERMUX_PREFIX/bin/llvm-ranlib"
    export LD="$TERMUX_PREFIX/bin/ld.lld"
    
    # Flags para Android usando sysroot de Termux (ya es bionic ARM64)
    export CFLAGS="--target=aarch64-linux-android$ANDROID_API \
        --sysroot=$ANDROID_SYSROOT \
        -fPIC -fPIE \
        -D__ANDROID__ -DANDROID \
        -I$TERMUX_PREFIX/include"
    
    # filesystem está en libc++ desde C++17, forzar feature test
    export CXXFLAGS="$CFLAGS -frtti -fexceptions -std=c++20 -D_LIBCPP_HAS_FILESYSTEM=1 -D_LIBCPP_HAS_EXPERIMENTAL_FILESYSTEM=1"
    # -llog, -ldl, -lm se resuelven en runtime en Android (bionic libc)
    export LDFLAGS="--target=aarch64-linux-android$ANDROID_API \
        --sysroot=$ANDROID_SYSROOT \
        -fPIC -fPIE -fuse-ld=lld \
        -L$TERMUX_PREFIX/lib \
        -landroid -landroid-support -latomic -lc++_shared"
    
    export PKG_CONFIG_PATH="$TERMUX_PREFIX/lib/pkgconfig"
    export PKG_CONFIG_SYSROOT_DIR="$ANDROID_SYSROOT"
    
    ok "Toolchain Termux nativo: $CC"
}

# =============================================================================
# VERIFICAR SYSROOT NATIVO DE TERMUX (no necesita NDK)
# =============================================================================
verify_termux_sysroot() {
    log "Verificando sysroot nativo de Termux..."
    
    # Termux ya tiene headers y libs para Android ARM64
    # libdl, libm, liblog son parte de bionic libc en Android (runtime)
    local required=(
        "$TERMUX_PREFIX/include/android/log.h"
        "$TERMUX_PREFIX/include/jni.h"
        "$TERMUX_PREFIX/lib/libc++_shared.so"
        "$TERMUX_PREFIX/lib/libandroid-support.so"
    )
    
    for f in "${required[@]}"; do
        [[ -f "$f" ]] || error "Falta: $f (ejecuta: pkg install libc++ libandroid-support)"
    done
    
    ok "Sysroot Termux verificado"
}

# =============================================================================
# BUILD OPENSSL/CURL/NGTCP2 PARA ANDROID ARM64 (usando Termux packages)
# =============================================================================
build_deps_termux() {
    log "Verificando dependencias nativas (OpenSSL, curl, etc.)..."
    
    # Termux ya tiene estos paquetes compilados para Android ARM64
    # Solo necesitamos asegurar que pkg-config los encuentre
    
    local deps=(openssl libcurl nghttp2 zlib)
    for dep in "${deps[@]}"; do
        if pkg-config --exists "$dep" 2>/dev/null; then
            ok "  $dep: $(pkg-config --modversion $dep)"
        else
            warn "  $dep no encontrado en pkg-config"
        fi
    done
    
    # ngtcp2/nghttp3 no están en Termux, compilar manualmente si se necesitan
    # Para HTTP/3, por ahora usar curl con OpenSSL (HTTP/2)
}

# =============================================================================
# CMAKE PARA TERMUX (usa sysroot nativo de Termux)
# =============================================================================
configure_cmake() {
    log "Configurando CMake para Termux ARM64 nativo..."
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Toolchain file para Termux nativo (sin NDK)
    cat > toolchain_termux.cmake << 'EOF'
# Toolchain para Termux nativo Android ARM64
set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_VERSION 24)
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
set(CMAKE_ANDROID_STL_TYPE c++_shared)

# Compiladores de Termux
set(CMAKE_C_COMPILER /data/data/com.termux/files/usr/bin/clang)
set(CMAKE_CXX_COMPILER /data/data/com.termux/files/usr/bin/clang++)
set(CMAKE_AR /data/data/com.termux/files/usr/bin/llvm-ar)
set(CMAKE_RANLIB /data/data/com.termux/files/usr/bin/llvm-ranlib)
set(CMAKE_STRIP /data/data/com.termux/files/usr/bin/llvm-strip)
set(CMAKE_LINKER /data/data/com.termux/files/usr/bin/ld.lld)

# Flags - usar sysroot de Termux (ya es Android ARM64 bionic)
set(CMAKE_C_FLAGS "--target=aarch64-linux-android24 --sysroot=/data/data/com.termux/files/usr -fPIC -fPIE -D__ANDROID__ -DANDROID" CACHE STRING "" FORCE)
# filesystem en libc++ - también para try_compile
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -frtti -fexceptions -std=c++20 -D_LIBCPP_HAS_FILESYSTEM=1 -D_LIBCPP_HAS_EXPERIMENTAL_FILESYSTEM=1" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "" FORCE)
# IMPORTANTE: flags para try_compile (usado por subproyectos como ixwebsocket)
set(CMAKE_TRY_COMPILE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_TRY_COMPILE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "" FORCE)
# Forzar arquitectura para Dobby y otros checks
set(CMAKE_SYSTEM_PROCESSOR "aarch64" CACHE STRING "" FORCE)
set(CMAKE_ANDROID_ARCH_ABI "arm64-v8a" CACHE STRING "" FORCE)
# -llog, -ldl, -lm se resuelven en runtime en Android (bionic libc)
set(CMAKE_EXE_LINKER_FLAGS "--target=aarch64-linux-android24 --sysroot=/data/data/com.termux/files/usr -fPIC -fPIE -fuse-ld=lld -landroid -landroid-support -latomic -lc++_shared")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

# Find root en Termux
set(CMAKE_FIND_ROOT_PATH /data/data/com.termux/files/usr)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
EOF
    
    # Ejecutar CMake con el toolchain
    cmake -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$BUILD_DIR/toolchain_termux.cmake" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$ARTIFACTS_DIR/install" \
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
        -DCPR_USE_SYSTEM_CURL=ON \
        -DCPR_ENABLE_SSL=OPENSSL \
        -DCMAKE_CXX_STANDARD=20 \
        -DCMAKE_CXX_FLAGS="-frtti -fexceptions -std=c++20 -D_LIBCPP_HAS_FILESYSTEM=1 -D_LIBCPP_HAS_EXPERIMENTAL_FILESYSTEM=1" \
        "$PROJECT_DIR"
    
    # Parchear CPR para saltar check de filesystem (try_compile no hereda flags)
    if [[ -f "$BUILD_DIR/_deps/cpr-src/include/CMakeLists.txt" ]]; then
        log "Parcheando CPR para saltar check de filesystem..."
        sed -i 's/if(NOT CPR_FILESYSTEM)/if(TRUE)/' "$BUILD_DIR/_deps/cpr-src/include/CMakeLists.txt" 2>/dev/null || true
        sed -i '/try_compile.*filesystem/d' "$BUILD_DIR/_deps/cpr-src/include/CMakeLists.txt" 2>/dev/null || true
        # Forzar definición de CPR_FILESYSTEM
        sed -i '/set(CPR_FILESYSTEM/a set(CPR_FILESYSTEM 1 CACHE INTERNAL "")' "$BUILD_DIR/_deps/cpr-src/include/CMakeLists.txt" 2>/dev/null || true
    fi
    
    # Re-ejecutar CMake para que tome el parche
    cmake -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$BUILD_DIR/toolchain_termux.cmake" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$ARTIFACTS_DIR/install" \
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
        -DCPR_USE_SYSTEM_CURL=ON \
        -DCPR_ENABLE_SSL=OPENSSL \
        -DCMAKE_CXX_STANDARD=20 \
        -DCMAKE_CXX_FLAGS="-frtti -fexceptions -std=c++20 -D_LIBCPP_HAS_FILESYSTEM=1 -D_LIBCPP_HAS_EXPERIMENTAL_FILESYSTEM=1" \
        "$PROJECT_DIR"
    
    ok "CMake configurado"
}

# =============================================================================
# BUILD NATIVO
# =============================================================================
build_native() {
    log "Compilando librería nativa..."
    cd "$BUILD_DIR"
    
    # Compilar con ninja
    ninja -j$(nproc) MobileBlox 2>&1 | tee build.log
    
    # Verificar resultado
    if [[ -f "libMobileBlox.so" ]]; then
        ok "libMobileBlox.so compilado"
        file libMobileBlox.so
        # Verificar que es ARM64
        readelf -h libMobileBlox.so | grep -q "AArch64" && ok "Confirmado: ARM64 (AArch64)"
    else
        error "Falló la compilación - revisa build.log"
    fi
}

# =============================================================================
# BUILD APK CON GRADLE TERMUX
# =============================================================================
build_apk_termux() {
    log "Construyendo APK con Gradle de Termux..."
    cd "$PROJECT_DIR"
    
    # Configurar local.properties para Termux
    cat > local.properties << EOF
sdk.dir=$TERMUX_PREFIX/opt/android-sdk
ndk.dir=$TERMUX_PREFIX/opt/android-ndk
# O si no hay SDK/NDK en Termux, usar los del sistema
EOF
    
    # Verificar si Termux tiene Android SDK
    if [[ ! -d "$TERMUX_PREFIX/opt/android-sdk" ]]; then
        log "Instalando Android SDK en Termux..."
        mkdir -p "$TERMUX_PREFIX/opt/android-sdk/cmdline-tools"
        wget -q "https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip" -O /tmp/sdk.zip
        unzip -q /tmp/sdk.zip -d "$TERMUX_PREFIX/opt/android-sdk/cmdline-tools"
        mv "$TERMUX_PREFIX/opt/android-sdk/cmdline-tools/cmdline-tools" "$TERMUX_PREFIX/opt/android-sdk/cmdline-tools/latest"
        rm /tmp/sdk.zip
        
        yes | "$TERMUX_PREFIX/opt/android-sdk/cmdline-tools/latest/bin/sdkmanager" --licenses
        "$TERMUX_PREFIX/opt/android-sdk/cmdline-tools/latest/bin/sdkmanager" \
            "platforms;android-34" \
            "build-tools;34.0.0" \
            "platform-tools" \
            "ndk;26.1.10909125"
    fi
    
    # Copiar lib nativa a jniLibs para Gradle
    mkdir -p app/src/main/jniLibs/arm64-v8a
    cp "$BUILD_DIR/libMobileBlox.so" app/src/main/jniLibs/arm64-v8a/
    
    # Build APK
    chmod +x gradlew
    ./gradlew assembleRelease --no-daemon \
        -Dorg.gradle.jvmargs="-Xmx2g" \
        -Pandroid.ndkPath="$TERMUX_PREFIX/opt/android-ndk" \
        2>&1 | tee gradle_build.log
    
    # Verificar APK
    local apk="app/build/outputs/apk/release/app-release.apk"
    if [[ -f "$apk" ]]; then
        ok "APK generado: $apk"
        unzip -l "$apk" | grep libMobileBlox.so
    else
        error "Falló build APK - revisa gradle_build.log"
    fi
}

# =============================================================================
# EMpaquetar ARTEFACTOS
# =============================================================================
package_artifacts() {
    log "Empaquetando artefactos..."
    mkdir -p "$ARTIFACTS_DIR"
    
    local date_str=$(date +%Y%m%d_%H%M%S)
    cp "app/build/outputs/apk/release/app-release.apk" "$ARTIFACTS_DIR/MobileBloxV2-S25FE-$date_str.apk"
    cp "$BUILD_DIR/libMobileBlox.so" "$ARTIFACTS_DIR/libMobileBlox-S25FE-$date_str.so"
    
    cat > "$ARTIFACTS_DIR/BUILD_INFO.txt" << EOF
MobileBlox V2 - S25 FE Native Build
===================================
Device: Samsung Galaxy S25 FE (ARM64)
Android: $(getprop ro.build.version.release) (API $(getprop ro.build.version.sdk))
Termux: $TERMUX_VERSION
Date: $(date)
Arch: $(uname -m)
Kernel: $(uname -r)
Git: $(git rev-parse --short HEAD 2>/dev/null || echo "N/A")
EOF
    
    ok "Artefactos en $ARTIFACTS_DIR:"
    ls -lh "$ARTIFACTS_DIR"
}

# =============================================================================
# INSTALAR EN DISPOSITIVO
# =============================================================================
install_apk() {
    local apk="$ARTIFACTS_DIR/MobileBloxV2-S25FE-$(date +%Y%m%d_%H%M%S).apk"
    # Buscar el APK más reciente
    apk=$(ls -t "$ARTIFACTS_DIR"/MobileBloxV2-S25FE-*.apk 2>/dev/null | head -1)
    
    if [[ -f "$apk" ]]; then
        log "Instalando APK en el dispositivo..."
        termux-open --send "$apk" 2>/dev/null || \
        adb install -r "$apk" 2>/dev/null || \
        pm install -r "$apk" 2>/dev/null || \
        warn "Instalación manual requerida: $apk"
    fi
}

# =============================================================================
# MAIN
# =============================================================================
main() {
    log "=========================================="
    log "MobileBlox V2 - Build Nativo Termux ARM64"
    log "Target: Samsung Galaxy S25 FE"
    log "=========================================="
    
    check_termux
    install_termux_deps
    setup_termux_toolchain
    verify_termux_sysroot
    build_deps_termux
    configure_cmake
    build_native
    build_apk_termux
    package_artifacts
    
    log "=========================================="
    ok "BUILD COMPLETADO EN S25 FE"
    log "=========================================="
    ok "APK: $ARTIFACTS_DIR/MobileBloxV2-S25FE-*.apk"
    ok "Lib: $ARTIFACTS_DIR/libMobileBlox-S25FE-*.so"
    
    if [[ "${1:-}" == "--install" ]]; then
        install_apk
    fi
}

main "$@"