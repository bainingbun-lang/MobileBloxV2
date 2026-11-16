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
