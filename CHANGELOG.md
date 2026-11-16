# MobileBloxV2 - Changelog

## [Unreleased] - Modernization Phase 1-9

### Phase 1: Audit & Architecture ✓
- Created PROJECT_INDEX.md
- Created ARCHITECTURE.md
- Created DEPENDENCIES.md
- Created BUILD.md
- Created CHANGELOG.md
- Created TODO.md

### Phase 2: ARM64 Compatibility [In Progress]
- [x] Add arm64-v8a to CMake configuration
- [ ] Rebuild all native dependencies for ARM64
- [x] Update NDK to r26d, Clang 17+, API 24
- [x] Migrate from vcxproj to CMake (root + NativeLib)
- [x] Create Gradle Android project (app module)
- [x] AndroidManifest, MainActivity, Theme, Resources
- [x] Luau luaconf.h patching for Roblox forwarders
- [ ] Verify JNI_OnLoad on ARM64

### Phase 3: Dependency Modernization
- [x] Luau as FetchContent subproject
- [x] ImGui as FetchContent (docking branch)
- [x] CPR as FetchContent
- [x] IXWebSocket as FetchContent
- [x] Dobby as FetchContent
- [x] LZ4 as FetchContent
- [x] nlohmann/json as FetchContent
- [ ] OpenSSL 3.2+ / curl 8.7+ / ngtcp2 stack (need vcpkg or manual build)
- [ ] Remove prebuilt .a files, use CMake FetchContent

### Phase 4: Visual Identity
- [x] Black background / Dark gray panels / Black buttons / White text / Electric blue accent
- [x] Logo: "_" in electric blue on black button (theme.hpp)
- [x] ImGui theme implementation (ApplyTheme)
- [ ] Custom font loading (JetBrains Mono)

### Phase 5: Luau Editor
- [x] ImGui-based code editor (editor.hpp/cpp)
- [x] Syntax highlighting (Luau keywords, types, globals)
- [x] Line numbers
- [x] Tab management
- [x] File operations (New, Open, Save, Close)
- [x] Console output area
- [x] Error diagnostics (markers in gutter)
- [ ] Autocomplete (LSP or local)
- [ ] Formatter (Luau CLI or built-in)
- [ ] Save/Load project files (.mbproj)
- [ ] Controlled execution environment

### Phase 6: Explorer
- [x] Hierarchical tree view (explorer.hpp/cpp)
- [x] Expandable folders
- [x] Object properties panel (Inspector)
- [x] Search/filter
- [x] Context menu (Create/Delete/Rename/Duplicate/Move)
- [x] Property inspector
- [x] Drag & drop support
- [x] Toggle visibility
- [x] Project resource display (real filesystem)

### Phase 7: AI Assistant
- [x] HTTP endpoint configuration UI (ai_assistant.hpp/cpp)
- [x] Adapters: Ollama, OpenAI, Anthropic, Custom (OpenAI-compatible)
- [x] Chat interface with history
- [x] Streaming responses (SSE parsing)
- [x] File context injection
- [x] Compiler error analysis
- [x] Code explanation/fix requests
- [x] Connection test button
- [ ] Claude Code adapter (needs proxy)

### Phase 8: Testing & Optimization
- [ ] Unit tests: Editor, Explorer, File IO, HTTP, AI adapters
- [ ] Integration tests: Native library load, JNI, Luau execution
- [ ] ARM64 device testing
- [ ] Memory profiling
- [ ] Performance benchmarks
- [ ] Stability testing (crash recovery)

### Phase 9: Compilation & Delivery
- [ ] Signed release APK
- [ ] All native libs ARM64
- [ ] Complete documentation
- [ ] Test results report
- [ ] Feature checklist

---

## Legacy Versions

### [2.0.0] - Original Release
- ARM32 only (armeabi-v7a)
- Visual Studio + NDK r21e + Clang 5.0
- Hardcoded Roblox offsets (v2.603.563)
- Basic Luau execution via ScriptContext hook
- Roblox Lua-based UI (LuaUI.hpp)
- Dobby hooking
- ImGui included but unused
- No Android app module (native lib only)