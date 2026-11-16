# MobileBloxV2 - TODO Tracker

## Phase 1: Audit & Architecture ✓ COMPLETE
- [x] Analyze all directories and source files
- [x] Document project structure (PROJECT_INDEX.md)
- [x] Document architecture (ARCHITECTURE.md)
- [x] Document dependencies (DEPENDENCIES.md)
- [x] Document build process (BUILD.md)
- [x] Create changelog (CHANGELOG.md)
- [x] Create this TODO tracker

## Phase 2: ARM64 Compatibility 🔄 IN PROGRESS

### CMake Migration
- [x] Create root CMakeLists.txt
- [x] Create NativeLib/CMakeLists.txt
- [x] Configure Android toolchain
- [x] Add arm64-v8a target
- [x] Add armeabi-v7a target (optional)
- [x] Set C++20 standard
- [x] Configure STL: c++_shared
- [x] Set API level 24

### Dependency Build System
- [x] Luau as CMake subproject (FetchContent)
- [x] ImGui as CMake subproject (docking branch)
- [x] CPR as CMake subproject
- [x] IXWebSocket as CMake subproject
- [x] Dobby as CMake subproject
- [x] LZ4 as CMake subproject
- [x] nlohmann/json as CMake subproject
- [ ] OpenSSL + curl + ngtcp2 stack (vcpkg or manual CMake)

### Gradle Android Project
- [x] Create app/ module
- [x] build.gradle.kts with NDK config
- [x] CMake integration
- [x] JNI_OnLoad registration
- [x] Manifest + permissions
- [x] Signing config
- [x] Theme (Black/Dark Gray/Black Buttons/White Text/Electric Blue)
- [x] MainActivity, Toolbar, FAB, Fragment placeholder
- [x] Vector drawable icons (Logo "_", File, Folder, Explorer, Console, AI, Settings, Play)

### Verification
- [ ] Build libMobileBlox.so for arm64-v8a
- [ ] Test JNI_OnLoad on ARM64 device/emulator
- [ ] Verify dobby hooks work ARM64
- [ ] Verify Luau forwarders work ARM64

## Phase 3: Dependency Modernization

### Luau
- [x] FetchContent integration
- [x] Forwarders isolated in roblox_forwarders.cpp
- [x] luaconf.h patching via CMake custom command
- [ ] Update to latest Luau version (0.635+)
- [ ] Ensure bytecode compatibility

### HTTP/WebSocket Stack
- [x] CPR + IXWebSocket FetchContent
- [ ] Rebuild curl with OpenSSL + ngtcp2 for ARM64
- [ ] Verify CPR works with new curl
- [ ] Verify IXWebSocket works

### Hooking
- [x] Dobby FetchContent
- [ ] Build Dobby from source ARM64
- [ ] Test inline hooks on ARM64

### UI Backend
- [x] ImGui docking branch FetchContent
- [ ] Add Vulkan backend (imgui_impl_vulkan.cpp)
- [ ] Keep OpenGL ES 3 backend

## Phase 4: Visual Identity

### Theme System
- [x] ImGui style: Black bg (#000000), Dark panels (#1a1a1a), Black buttons (#0d0d0d), White text (#ffffff), Electric blue (#00d4ff)
- [ ] Custom font: JetBrains Mono or similar
- [x] Logo rendering: "_" in electric blue on black button (RenderLogo)
- [ ] Theme persistence

### UI Framework
- [x] Main window with menu bar
- [x] Docking layout (ImGui docking) - SetupDockingLayout
- [x] Panels: Editor, Explorer, Console, AI Chat
- [ ] Responsive layout

## Phase 5: Luau Editor

### Core Editor
- [x] Custom ImGui-based editor (editor.hpp/cpp)
- [x] Luau syntax highlighting (keywords, types, globals, strings, numbers, comments)
- [x] Line numbers + gutter
- [x] Tab bar (multiple files)
- [x] File operations: New, Open, Save, Save As, Close

### Advanced Features
- [ ] Autocomplete (LSP client or local keyword DB)
- [x] Error diagnostics (markers in gutter)
- [ ] Formatter integration (luau fmt)
- [ ] Find/Replace
- [ ] Go to line
- [ ] Bracket matching
- [ ] Minimap (optional)

### Execution
- [x] "Run" button → callback to native execution queue
- [ ] Console output capture
- [ ] Execution status indicator
- [ ] Stop/Kill running script

### Project Management
- [ ] Project file format (.mbproj JSON)
- [x] File tree sidebar (via Explorer)
- [ ] Recent projects
- [ ] Auto-save

## Phase 6: Explorer

### Tree View
- [x] Hierarchical node rendering
- [x] Expand/collapse folders
- [x] Icons per type (Folder, Script, ModuleScript, LocalScript, etc.)
- [x] Search/filter bar
- [x] Multi-select (Ctrl+Click)

### Property Inspector
- [x] Selected object properties (Name, Type, Path, Size, Modified)
- [ ] Editable fields (Name, Parent, etc.)
- [ ] Type-appropriate editors (color picker, vector3, etc.)
- [ ] Read-only vs writable distinction

### Context Menu
- [x] Create: Script, ModuleScript, Folder
- [x] Delete
- [x] Rename
- [x] Duplicate
- [x] Copy Path
- [x] Move (Drag & Drop)

### Integration
- [x] Toggle visibility via menu
- [x] Sync with Editor tabs (double-click opens in editor)
- [x] Show project resources only (real filesystem)

## Phase 7: AI Assistant

### Configuration UI
- [x] Endpoint URL input
- [x] Model selector
- [x] API Key field (password mode)
- [x] Timeout input
- [x] Connection status indicator
- [x] "Test Connection" button

### Adapter System
- [x] Base Adapter interface (AIAdapter)
- [ ] Claude Code adapter (Anthropic API) - needs proxy
- [x] Ollama adapter (/api/chat, /api/tags)
- [x] OpenAI-compatible adapter (OpenAI, Custom)
- [x] Anthropic adapter (Claude)
- [x] Auto-detect from endpoint (ProviderToString)

### Chat Interface
- [x] Message history (persistent in session)
- [x] Streaming response rendering (SSE)
- [ ] Code block rendering with copy button
- [x] Context menu actions: Send file, Send error, Explain, Fix

### Context Injection
- [x] Current file content (SetCurrentFileContext)
- [ ] Selection range
- [x] Compiler errors (SendErrorForAnalysis)
- [ ] Project structure summary

## Phase 8: Testing & Optimization

### Unit Tests
- [ ] Editor: syntax highlight, autocomplete, file ops
- [ ] Explorer: tree ops, property edit, context menu
- [ ] File IO: save/load, project format
- [ ] HTTP: request/response, timeout, error handling
- [ ] AI Adapters: each provider, streaming, errors
- [ ] Native: JNI load, hook install, Luau exec

### Integration Tests
- [ ] Full script execution flow
- [ ] Explorer ↔ Editor sync
- [ ] AI error analysis flow
- [ ] APK install + run on device

### Performance
- [ ] Memory profiling (valgrind/asan)
- [ ] Startup time
- [ ] Script compilation time
- [ ] UI frame time (ImGui)
- [ ] Native lib size

### Stability
- [ ] Crash handler (catch SIGSEGV, log)
- [ ] Graceful degradation (AI offline)
- [ ] Hook failure recovery
- [ ] Memory leak detection

## Phase 9: Compilation & Delivery

### Release Build
- [ ] Gradle release config (minify, shrink)
- [ ] Signing (keystore)
- [ ] APK + AAB outputs
- [ ] Native libs stripped

### Documentation
- [ ] USER_GUIDE.md
- [ ] DEVELOPER_GUIDE.md
- [ ] API_REFERENCE.md
- [ ] CONFIGURATION.md

### Final Verification
- [ ] Test on Android 10, 12, 14, 15
- [ ] Test on ARM64 devices
- [ ] Verify all features work
- [ ] Performance benchmarks recorded

---

## Blockers / Risks

| Risk | Impact | Mitigation |
|------|--------|------------|
| Roblox offset changes | Critical | Dynamic resolution, version detection |
| SELinux blocks memory ops | High | Magisk module, patch SELinux policy |
| Luau bytecode drift | High | Track Luau upstream, test compatibility |
| NDK API changes | Medium | Pin NDK version, test upgrades |
| Dobby ARM64 bugs | Medium | Test extensively, have Frida fallback |