cmake_minimum_required(VERSION 3.22)
project(MobileBloxV2 VERSION 2.0.0 LANGUAGES CXX C)

# =============================================================================
# CONFIGURATION OPTIONS
# =============================================================================
option(BUILD_ARM64 "Build for arm64-v8a" ON)
option(BUILD_ARM32 "Build for armeabi-v7a" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(MOBILEBLOX_BUILD_TESTS "Build unit tests" OFF)

# Android-only project
if(NOT ANDROID)
    message(FATAL_ERROR "This project must be built with Android NDK toolchain. Set CMAKE_TOOLCHAIN_FILE to \$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake")
endif()

# =============================================================================
# ANDROID TOOLCHAIN SETTINGS (set via toolchain file, but defaults here)
# =============================================================================
if(NOT ANDROID_ABI)
    if(BUILD_ARM64)
        set(ANDROID_ABI "arm64-v8a" CACHE STRING "Target ABI" FORCE)
    elseif(BUILD_ARM32)
        set(ANDROID_ABI "armeabi-v7a" CACHE STRING "Target ABI" FORCE)
    else()
        set(ANDROID_ABI "arm64-v8a" CACHE STRING "Target ABI" FORCE)
    endif()
endif()

if(NOT ANDROID_PLATFORM)
    set(ANDROID_PLATFORM "android-24" CACHE STRING "Minimum API level" FORCE)
endif()

if(NOT ANDROID_STL)
    set(ANDROID_STL "c++_shared" CACHE STRING "STL implementation" FORCE)
endif()

if(NOT ANDROID_TOOLCHAIN)
    set(ANDROID_TOOLCHAIN "clang" CACHE STRING "Toolchain" FORCE)
endif()

# =============================================================================
# C++ STANDARD & COMPILER FLAGS
# =============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warning flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -Wno-missing-field-initializers
        -Wno-sign-compare
        -Wno-unused-variable
        -Wno-unused-function
    )
endif()

# Release optimizations
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")

# =============================================================================
# OUTPUT DIRECTORIES
# =============================================================================
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${ANDROID_ABI})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${ANDROID_ABI})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${ANDROID_ABI})

# =============================================================================
# FETCHCONTENT DEPENDENCIES
# =============================================================================
include(FetchContent)

# ---- Luau (Roblox fork) ----
FetchContent_Declare(
    luau
    GIT_REPOSITORY https://github.com/luau-lang/luau
    GIT_TAG        0.635  # Update to latest stable
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(luau)

# ---- Dear ImGui (docking branch) ----
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG        docking
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

# ---- CPR (C++ Requests) ----
# Temporalmente deshabilitado por issues con filesystem en try_compile
# Usar libcurl directo en el código
# FetchContent_Declare(
#     cpr
#     GIT_REPOSITORY https://github.com/libcpr/cpr
#     GIT_TAG        1.11.0
#     GIT_SHALLOW    TRUE
# )
# set(CPR_ENABLE_SSL OPENSSL)
# set(CPR_FORCE_USE_SYSTEM_CURL ON)
# FetchContent_MakeAvailable(cpr)

# ---- IXWebSocket ----
# Temporalmente deshabilitado por issues con libatomic en try_compile
# Usar HTTP polling en AI assistant
# FetchContent_Declare(
#     ixwebsocket
#     GIT_REPOSITORY https://github.com/machinezone/IXWebSocket
#     GIT_TAG        master
#     GIT_SHALLOW    TRUE
# )
# set(IXWEBSOCKET_USE_TLS ON)
# set(IXWEBSOCKET_BUILD_TESTS OFF)
# FetchContent_MakeAvailable(ixwebsocket)

# ---- Dobby (hooking) ----
# Temporalmente deshabilitado por check de arquitectura en CMake
# Compilar libdobby.a por separado para ARM64
# FetchContent_Declare(
#     dobby
#     GIT_REPOSITORY https://github.com/jmpews/Dobby
#     GIT_TAG        master
#     GIT_SHALLOW    TRUE
# )
# FetchContent_MakeAvailable(dobby)

# ---- LZ4 ----
FetchContent_Declare(
    lz4
    GIT_REPOSITORY https://github.com/lz4/lz4
    GIT_TAG        v1.10.0
    GIT_SHALLOW    TRUE
)
set(LZ4_BUILD_CLI OFF)
set(LZ4_BUILD_LEGACY_LZ4C OFF)
set(LZ4_PROGRAMS OFF)
set(LZ4_BUILD_TESTS OFF)
FetchContent_MakeAvailable(lz4)

# ---- nlohmann/json (for AI Assistant) ----
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG        v3.11.2
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# =============================================================================
# OPENSSL + CURL + NGTCP2 (via system/NDK or vcpkg)
# =============================================================================
# These are typically provided by NDK or built separately.
# For now, find_package - user must provide via CMAKE_PREFIX_PATH or NDK
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# ngtcp2/nghttp3/nghttp2 - try to find, else build from source
# TODO: Add FetchContent for these if not found

# =============================================================================
# INCLUDE SUBDIRECTORIES
# =============================================================================
add_subdirectory(NativeLib)

# =============================================================================
# INSTALL TARGETS (for Gradle integration)
# =============================================================================
install(
    TARGETS MobileBlox
    LIBRARY DESTINATION ${ANDROID_ABI}
)