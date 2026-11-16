# MobileBloxV2 - Project Index

## Overview
Roblox Android script executor (32-bit ARM) built with Visual Studio + Android NDK.
Target: ARM64 modernization + Luau IDE + AI integration.

## Directory Structure
```
MobileBloxV2-main/
├── MobileBloxV2.sln              # VS Solution (ARM only)
├── NativeLib/
│   ├── MobileBloxV2.vcxproj      # Project config (ARM, Clang 5.0, API 21)
│   ├── main.cpp                  # JNI_OnLoad entry point
│   ├── pch.hpp                   # Precompiled header, logging, config
│   ├── LuaUI.hpp                 # Embedded Roblox Lua GUI
│   ├── exploit.hpp/cpp           # Exploit lifecycle
│   ├── execution/                # Luau bytecode compilation + execution
│   ├── renderer/                 # Renderer stub (Vulkan hook placeholder)
│   ├── roblox/
│   │   ├── hooks.hpp/cpp         # Roblox function hooks
│   │   ├── update.hpp/cpp        # Addresses, offsets, structs, function pointers
│   ├── taskscheduler/            # TaskScheduler interaction
│   ├── reflection/               # Instance reflection
│   ├── environment/              # Luau environment libraries
│   ├── callcheck/                # Call validation
│   ├── utils/                    # Memory, JNI, LZ4, random
│   └── dependencies/
│       ├── luau/                 # Luau VM (Ast, Compiler, VM, Common)
│       ├── imgui/                # Dear ImGui + Android/OpenGL3 backends
│       ├── cpr/                  # C++ Requests (HTTP)
│       ├── curl/                 # libcurl + OpenSSL/ngtcp2 (armeabi-v7a .a)
│       ├── ixwebsocket/          # WebSocket client/server
│       ├── dobby/                # Hooking library (armeabi-v7a .a)
│       ├── lz4/                  # Compression
│       └── base64.hpp, HttpStatus.hpp
├── libc++_shared.so              # STL runtime
└── gdbserver                     # Debug server
```

## Key Components

### Native Layer (C++)
- **JNI_OnLoad** → waits for `libroblox.so` → initializes exploit
- **Exploit**: Hooks Roblox internals via dobby, custom Luau VM functions
- **Execution**: Compiles Luau → custom bytecode encoder (opcode ×227) → schedules via ScriptContext
- **TaskScheduler**: Finds jobs, hooks ScriptContext, manages thread identity
- **Environment**: Registers custom libraries (misc, closure, debug, websocket, reflection, etc.)
- **Renderer**: Stub only (Vulkan hook planned)

### Dependencies (Prebuilt ARM32)
| Library | Arch | Type |
|---------|------|------|
| curl, crypto, ssl, nghttp2, nghttp3, ngtcp2 | armeabi-v7a | Static .a |
| dobby | armeabi-v7a | Static .a |
| libc++_shared.so | armeabi-v7a | Shared |

### Luau VM Modifications
- `luaconf.h`: Function pointers to Roblox's internal Luau functions
- `lobject.h`: `luaO_nilobject` rebased
- `ltable.cpp`: `dummynode` rebased
- `lvmexecute.cpp`, `lvmutils.cpp`, `ldo.cpp`, `lgc.cpp`: Forwarders to Roblox Luau

## Build Configuration
- **Toolset**: Clang 5.0 (ancient)
- **API Level**: android-21
- **STL**: c++_shared
- **C++ Standard**: c++1z (C++17)
- **Output**: `libMobileBlox.so` (ARM 32-bit only)

## Current Limitations
- ARM32 only (no arm64-v8a)
- NDK toolchain outdated (Clang 5.0)
- No CMake support
- Hardcoded Roblox version offsets (2.603.563)
- No actual Android app module (only native lib)
- ImGui unused in current UI (uses Roblox Lua UI)
- No editor, explorer, or AI features