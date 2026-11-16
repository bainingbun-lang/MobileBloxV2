# NativeLib/CMakeLists.txt
# MobileBloxV2 Native Library Build Configuration

# =============================================================================
# LUACONF.H PATCHING - Patch Luau's luaconf.h after FetchContent
# =============================================================================
# We patch the actual Luau source file to ensure it's used during Luau's build
# This runs after FetchContent_MakeAvailable(luau) in parent CMakeLists.txt

# Custom command to patch luaconf.h in Luau source
add_custom_command(
    OUTPUT ${luau_SOURCE_DIR}/VM/include/luaconf.h.patched
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/luaconf.h.in
        ${luau_SOURCE_DIR}/VM/include/luaconf.h
    COMMENT "Patching Luau luaconf.h for Roblox integration"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/luaconf.h.in
)

# Custom target to ensure patching happens before build
add_custom_target(patch_luau_luaconf DEPENDS ${luau_SOURCE_DIR}/VM/include/luaconf.h.patched)

# Make MobileBlox depend on patching
# (will be added via add_dependencies after MobileBlox target is created)

# =============================================================================
# SOURCE FILES
# =============================================================================
set(MOBILEBLOX_SOURCES
    # Core (fuentes originales en ../MobileBloxV2-main/NativeLib/)
    ../MobileBloxV2-main/NativeLib/main.cpp
    ../MobileBloxV2-main/NativeLib/exploit.cpp

    # Execution
    ../MobileBloxV2-main/NativeLib/execution/execution.cpp

    # Renderer
    ../MobileBloxV2-main/NativeLib/renderer/renderer.cpp

    # Roblox
    ../MobileBloxV2-main/NativeLib/roblox/hooks.cpp
    ../MobileBloxV2-main/NativeLib/roblox/update.cpp

    # TaskScheduler
    ../MobileBloxV2-main/NativeLib/taskscheduler/taskscheduler.cpp

    # Reflection
    ../MobileBloxV2-main/NativeLib/reflection/instance.cpp

    # Environment
    ../MobileBloxV2-main/NativeLib/environment/environment.cpp
    ../MobileBloxV2-main/NativeLib/environment/libraries/closure.cpp
    ../MobileBloxV2-main/NativeLib/environment/libraries/debug.cpp
    ../MobileBloxV2-main/NativeLib/environment/libraries/misc.cpp
    ../MobileBloxV2-main/NativeLib/environment/libraries/mthooks.cpp
    ../MobileBloxV2-main/NativeLib/environment/libraries/reflection.cpp
    ../MobileBloxV2-main/NativeLib/environment/libraries/websocket.cpp

    # CallCheck
    ../MobileBloxV2-main/NativeLib/callcheck/callcheck.cpp

    # Utils
    ../MobileBloxV2-main/NativeLib/utils/utils.cpp

    # Mis nuevos fuentes (en este mismo directorio)
    editor/editor.cpp
    explorer/explorer.cpp
    ai/ai_assistant.cpp

    # Dependencies (source-based)
    ../MobileBloxV2-main/NativeLib/dependencies/lz4/lz4.cpp
)

# Headers para precompiled header
set(MOBILEBLOX_HEADERS
    ../MobileBloxV2-main/NativeLib/pch.hpp
    ../MobileBloxV2-main/NativeLib/ui/theme.hpp
)

# =============================================================================
# TARGET DEFINITION
# =============================================================================
add_library(MobileBlox SHARED ${MOBILEBLOX_SOURCES})

# Output name matches original: libMobileBlox.so
set_target_properties(MobileBlox PROPERTIES
    OUTPUT_NAME "MobileBlox"
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    ANDROID_SO_NAME "libMobileBlox.so"
)

# Luau sources (from FetchContent)
set(LUAU_SOURCES
    ${luau_SOURCE_DIR}/Ast/src/Ast.cpp
    ${luau_SOURCE_DIR}/Ast/src/Confusables.cpp
    ${luau_SOURCE_DIR}/Ast/src/Lexer.cpp
    ${luau_SOURCE_DIR}/Ast/src/Location.cpp
    ${luau_SOURCE_DIR}/Ast/src/Parser.cpp
    ${luau_SOURCE_DIR}/Ast/src/StringUtils.cpp
    ${luau_SOURCE_DIR}/Ast/src/TimeTrace.cpp

    ${luau_SOURCE_DIR}/Compiler/src/BuiltinFolding.cpp
    ${luau_SOURCE_DIR}/Compiler/src/Builtins.cpp
    ${luau_SOURCE_DIR}/Compiler/src/BytecodeBuilder.cpp
    ${luau_SOURCE_DIR}/Compiler/src/Compiler.cpp
    ${luau_SOURCE_DIR}/Compiler/src/ConstantFolding.cpp
    ${luau_SOURCE_DIR}/Compiler/src/CostModel.cpp
    ${luau_SOURCE_DIR}/Compiler/src/TableShape.cpp
    ${luau_SOURCE_DIR}/Compiler/src/Types.cpp
    ${luau_SOURCE_DIR}/Compiler/src/ValueTracking.cpp
    ${luau_SOURCE_DIR}/Compiler/src/lcode.cpp

    ${luau_SOURCE_DIR}/VM/src/lapi.cpp
    ${luau_SOURCE_DIR}/VM/src/laux.cpp
    ${luau_SOURCE_DIR}/VM/src/lbaselib.cpp
    ${luau_SOURCE_DIR}/VM/src/lbitlib.cpp
    ${luau_SOURCE_DIR}/VM/src/lbuffer.cpp
    ${luau_SOURCE_DIR}/VM/src/lbuflib.cpp
    ${luau_SOURCE_DIR}/VM/src/lbuiltins.cpp
    ${luau_SOURCE_DIR}/VM/src/lcorolib.cpp
    ${luau_SOURCE_DIR}/VM/src/ldblib.cpp
    ${luau_SOURCE_DIR}/VM/src/ldebug.cpp
    ${luau_SOURCE_DIR}/VM/src/ldo.cpp
    ${luau_SOURCE_DIR}/VM/src/lfunc.cpp
    ${luau_SOURCE_DIR}/VM/src/lgc.cpp
    ${luau_SOURCE_DIR}/VM/src/lgcdebug.cpp
    ${luau_SOURCE_DIR}/VM/src/linit.cpp
    ${luau_SOURCE_DIR}/VM/src/lmathlib.cpp
    ${luau_SOURCE_DIR}/VM/src/lmem.cpp
    ${luau_SOURCE_DIR}/VM/src/lnumprint.cpp
    ${luau_SOURCE_DIR}/VM/src/lobject.cpp
    ${luau_SOURCE_DIR}/VM/src/loslib.cpp
    ${luau_SOURCE_DIR}/VM/src/lperf.cpp
    ${luau_SOURCE_DIR}/VM/src/lstate.cpp
    ${luau_SOURCE_DIR}/VM/src/lstring.cpp
    ${luau_SOURCE_DIR}/VM/src/lstrlib.cpp
    ${luau_SOURCE_DIR}/VM/src/ltable.cpp
    ${luau_SOURCE_DIR}/VM/src/ltablib.cpp
    ${luau_SOURCE_DIR}/VM/src/ltm.cpp
    ${luau_SOURCE_DIR}/VM/src/ludata.cpp
    ${luau_SOURCE_DIR}/VM/src/lutf8lib.cpp
    ${luau_SOURCE_DIR}/VM/src/lvmexecute.cpp
    ${luau_SOURCE_DIR}/VM/src/lvmload.cpp
    ${luau_SOURCE_DIR}/VM/src/lvmutils.cpp
)

# ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_android.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# CPR sources (already built as target via FetchContent)
# IXWebSocket sources (already built as target via FetchContent)

# =============================================================================
# INCLUDE DIRECTORIES
# =============================================================================
target_include_directories(MobileBlox PRIVATE
    # Current project
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/execution
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/roblox
    ${CMAKE_CURRENT_SOURCE_DIR}/taskscheduler
    ${CMAKE_CURRENT_SOURCE_DIR}/reflection
    ${CMAKE_CURRENT_SOURCE_DIR}/environment
    ${CMAKE_CURRENT_SOURCE_DIR}/environment/libraries
    ${CMAKE_CURRENT_SOURCE_DIR}/callcheck
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/editor
    ${CMAKE_CURRENT_SOURCE_DIR}/explorer
    ${CMAKE_CURRENT_SOURCE_DIR}/ai

    # Luau
    ${luau_SOURCE_DIR}/Ast/include
    ${luau_SOURCE_DIR}/Compiler/include
    ${luau_SOURCE_DIR}/VM/include
    ${luau_SOURCE_DIR}/Common/include

    # ImGui
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends

    # nlohmann/json
    ${nlohmann_json_SOURCE_DIR}/include

    # Dependencies (local)
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lz4
)

# System includes (NDK)
target_include_directories(MobileBlox SYSTEM PRIVATE
    ${ANDROID_NDK_HOME}/sysroot/usr/include
    ${ANDROID_NDK_HOME}/sysroot/usr/include/${ANDROID_TOOLCHAIN_TRIPLE}
)

# OpenSSL, CURL, etc.
target_include_directories(MobileBlox SYSTEM PRIVATE
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# =============================================================================
# COMPILE DEFINITIONS
# =============================================================================
target_compile_definitions(MobileBlox PRIVATE
    # Luau config
    LUAU_ENABLE_ASSERT=1
    LUAU_USER_MEMORY=1

    # ImGui config
    IMGUI_IMPL_OPENGL_LOADER_CUSTOM=1
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
    IMGUI_DISABLE_OBSOLETE_KEYIO=1

    # Project config
    MOBILEBLOX_VERSION="${PROJECT_VERSION}"
    ROBLOX_EXPLOIT_VERSION="2.603.563"  # TODO: Make dynamic

    # Android
    __ANDROID__=1
)

# =============================================================================
# LINK LIBRARIES
# =============================================================================
target_link_libraries(MobileBlox PRIVATE
    # Luau (built from source)
    luau

    # ImGui sources (header-only lib, compilar directamente)
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_android.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp

    # CPR deshabilitado temporalmente - usar libcurl directo
    # cpr::cpr

    # IXWebSocket deshabilitado temporalmente
    # ixwebsocket::ixwebsocket

    # Dobby deshabilitado temporalmente
    # dobby::dobby

    # LZ4
    lz4

    # OpenSSL
    OpenSSL::SSL
    OpenSSL::Crypto

    # CURL
    ${CURL_LIBRARIES}

    # Android system libraries
    android
    log
    dl
    m
    c
    EGL
    GLESv3
)

# =============================================================================
# COMPILER FLAGS PER TARGET
# =============================================================================
target_compile_options(MobileBlox PRIVATE
    # Exception handling & RTTI (required by Luau/CPR)
    -fexceptions
    -frtti

    # Thread safety
    -pthread

    # Optimization
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Debug>:-O0 -g>
)

# =============================================================================
# PRECOMPILED HEADER - Deshabilitado temporalmente por issues con sysroot
# =============================================================================
# target_precompile_headers(MobileBlox PRIVATE
#     pch.hpp
# )

# =============================================================================
# ROBLOX FORWARDERS (separate file, not in Luau source)
# =============================================================================
# Create roblox_forwarders.cpp that implements the forwarders to Roblox's Luau
# This replaces the modified Luau source files approach
add_library(roblox_forwarders STATIC
    roblox_forwarders.cpp
)
target_include_directories(roblox_forwarders PRIVATE
    ${luau_SOURCE_DIR}/VM/include
    ${luau_SOURCE_DIR}/VM/src
)
target_link_libraries(MobileBlox PRIVATE roblox_forwarders)

# =============================================================================
# ANDROID SPECIFIC
# =============================================================================
if(ANDROID)
    # Link libc++_shared
    target_link_libraries(MobileBlox PRIVATE c++_shared)

    # JNI
    target_compile_definitions(MobileBlox PRIVATE JNI_VERSION=1_6)
endif()

# =============================================================================
# VERSION INFO (already set at target creation)
# =============================================================================
# set_target_properties already called at line 32

# =============================================================================
# DEPENDENCIES
# =============================================================================
# Ensure luaconf.h patching happens before building
add_dependencies(MobileBlox patch_luau_luaconf)
add_dependencies(roblox_forwarders patch_luau_luaconf)