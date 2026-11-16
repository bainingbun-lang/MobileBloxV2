# MobileBloxV2 - Architecture Document

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Android App Process                        │
├─────────────────────────────────────────────────────────────┤
│  Java/Kotlin Layer (Missing - needs creation)                │
│  └── System.loadLibrary("MobileBlox")                        │
├─────────────────────────────────────────────────────────────┤
│  JNI Bridge (main.cpp: JNI_OnLoad)                           │
│  └── JNIEnv* cached globally                                 │
├─────────────────────────────────────────────────────────────┤
│  Native Layer (libMobileBlox.so)                             │
│  ├── Exploit Core                                            │
│  │   ├── Memory: wait_for_lib, find_lib, rebase              │
│  │   ├── Hooking: Dobby (inline hooks)                       │
│  │   └── Roblox Offsets: Hardcoded per version               │
│  ├── Luau Execution Engine                                   │
│  │   ├── Compiler: Luau Ast → Bytecode                       │
│  │   ├── Encoder: Custom (opcode × 227)                      │
│  │   ├── Scheduler: ScriptQueue + YieldQueue                 │
│  │   └── VM Forwarders: Roblox's luaD_*, luaV_*, luaC_*     │
│  ├── TaskScheduler Interface                                 │
│  │   ├── Job enumeration                                     │
│  │   ├── ScriptContext hooking                               │
│  │   └── Identity management (set_identity)                  │
│  ├── Environment Libraries                                   │
│  │   ├── Base: misc, closure, debug, bit, connections       │
│  │   ├── Advanced: reflection, websocket, crypt, script     │
│  │   └── Metatable/cache/registry manipulation               │
│  ├── Renderer (Stub)                                         │
│  └── Lua UI Injection (LuaUI.hpp)                            │
├─────────────────────────────────────────────────────────────┤
│  Roblox Process (libroblox.so)                               │
│  └── Target of hooks / function calls                        │
└─────────────────────────────────────────────────────────────┘
```

## Data Flow: Script Execution

```
User Script (Lua)
       │
       ▼
Luau Compiler (Ast → Bytecode)
       │
       ▼
Custom Bytecode Encoder (opcode × 227)
       │
       ▼
execution::execute() / schedule()
       │
       ▼
ScriptQueue / YieldQueue
       │
       ▼
TaskScheduler Hook (ScriptContext resume)
       │
       ▼
Roblox Luau VM (via forwarders)
       │
       ▼
Execution in Roblox Context
```

## Key Classes & Singletons

| Class | Purpose | Pattern |
|-------|---------|---------|
| `exploit::execution` | Script compilation, queue, scheduling | Singleton |
| `taskscheduler` | Roblox TaskScheduler interaction | Singleton |
| `roblox::functions` | Function pointers to Roblox internals | Namespace + inline vars |
| `renderer` | Drawing/Vulkan hook (stub) | Namespace |
| `exploit::environment` | Luau library registration | Namespace |

## Memory & Hooking Strategy

1. **Library Loading**: `utils::memory::wait_for_lib("libroblox.so")` polls `/proc/self/maps`
2. **Rebasing**: All offsets are relative to `libroblox.so` base
3. **Hooking**: Dobby inline hooks on Roblox functions
4. **VM Forwarders**: Modified Luau source calls into Roblox's Luau via function pointers

## Threading Model

- **JNI_OnLoad**: Main thread
- **Script Execution**: Hooked via ScriptContext job → runs on Roblox's task scheduler threads
- **Queues**: `script_queue` (std::queue), `yield_queue` (std::queue) - accessed from hooked context
- **No explicit synchronization** in current code (race conditions likely)

## Security / Stability Concerns

| Issue | Severity | Location |
|-------|----------|----------|
| Hardcoded offsets | Critical | `update.hpp` |
| No ARM64 support | Critical | `vcxproj`, prebuilt libs |
| Race conditions on queues | High | `execution.hpp/cpp` |
| No error recovery | High | `main.cpp`, `exploit.cpp` |
| Ancient NDK (Clang 5.0) | High | `vcxproj` |
| API 21 minimum | Medium | `vcxproj` |
| STL c++_shared conflicts | Medium | `vcxproj` |
| No SELinux consideration | Medium | Memory operations |

## Required Modernization

1. **ARM64**: Add arm64-v8a configs, rebuild all deps
2. **NDK**: Upgrade to r26+/Clang 17+, API 24+
3. **Build System**: Migrate to CMake + Gradle
4. **Offsets**: Dynamic resolution or version detection
5. **Thread Safety**: Mutexes on queues, atomic flags
6. **App Module**: Create Android application with UI
7. **Renderer**: Implement Vulkan/ImGui overlay
8. **Editor/Explorer/AI**: New modules