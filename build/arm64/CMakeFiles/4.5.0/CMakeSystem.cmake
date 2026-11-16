set(CMAKE_HOST_SYSTEM "Android-37")
set(CMAKE_HOST_SYSTEM_NAME "Android")
set(CMAKE_HOST_SYSTEM_VERSION "37")
set(CMAKE_HOST_SYSTEM_PROCESSOR "aarch64")

set(_CMAKE_INPUT_TOOLCHAIN_FILE [==[/data/data/com.termux/files/home/android-ndk-r26d/build/cmake/android.toolchain.cmake]==])
set(_CMAKE_SYSTEM_TOOLCHAIN_FILE "/data/data/com.termux/files/home/android-ndk-r26d/build/cmake/android.toolchain.cmake")
include("${_CMAKE_SYSTEM_TOOLCHAIN_FILE}")

set(CMAKE_SYSTEM "Android-1")
set(CMAKE_SYSTEM_NAME "Android")
set(CMAKE_SYSTEM_VERSION "1")
set(CMAKE_SYSTEM_PROCESSOR "aarch64")

set(CMAKE_CROSSCOMPILING "TRUE")

set(CMAKE_SYSTEM_LOADED 1)
