# MobileBloxV2 - Architectural Decisions

## D001: Build System → CMake + Gradle
**Date**: 2026-11-15
**Decision**: Migrate from Visual Studio vcxproj to CMake + Gradle
**Rationale**:
- Cross-platform (Linux/macOS/Windows)
- Native Android support via NDK toolchain
- Standard for Android NDK development
- Better dependency management (FetchContent)
- CI/CD friendly
**Alternatives**: Bazel (complex), Meson (less Android support), Keep VS (ARM32 only)

## D002: Target Architecture → ARM64 Primary
**Date**: 2026-11-15
**Decision**: Target arm64-v8a as primary, ARM32 optional
**Rationale**:
- All modern Android devices are ARM64
- Google Play requires 64-bit
- Better performance, more registers
- 32-bit support dropping in future Android versions
**Alternatives**: Dual build (larger APK), ARM32 only (obsolete)

## D003: NDK Version → r26d (LTS)
**Date**: 2026-11-15
**Decision**: Use NDK r26d with Clang 17+, API 24+
**Rationale**:
- LTS release, stable
- Clang 17 supports C++20 fully
- API 24 (Android 7.0) covers >95% devices
- Better sanitizers, optimizations
**Alternatives**: r27 beta (unstable), r25c (older Clang)

## D004: Luau Integration → FetchContent + Patch File
**Date**: 2026-11-15
**Decision**: Use Luau as Git submodule/FetchContent, apply patches via CMake
**Rationale**:
- Track upstream updates
- Isolate Roblox forwarders in separate file
- Clean separation of concerns
- Reproducible builds
**Alternatives**: Vendored copy (hard to update), System package (version mismatch)

## D005: HTTP Stack → Keep CPR + curl + OpenSSL
**Date**: 2026-11-15
**Decision**: Rebuild existing stack for ARM64 rather than replace
**Rationale**:
- CPR provides nice C++ API
- curl + OpenSSL + ngtcp2 is industry standard
- IXWebSocket works well for WS
- Minimal code changes needed
**Alternatives**: cpp-httplib (no HTTP/2), Boost.Beast (heavy), cpp-rest (abandoned)

## D006: Hooking Library → Dobby (Build from Source)
**Date**: 2026-11-15
**Decision**: Build Dobby from source for ARM64
**Rationale**:
- Lightweight, header + single .cpp
- Active maintenance
- ARM64 support in recent versions
- Inline hooking works well
**Alternatives**: Frida-gum (heavy, overkill), LiteHook (less tested), PLTHook (limited)

## D007: UI Framework → ImGui with Docking
**Date**: 2026-11-15
**Decision**: Use ImGui (already in deps) with docking branch for IDE layout
**Rationale**:
- Already integrated in project
- Immediate mode = simple, performant
- Docking enables professional IDE layout
- Vulkan backend for future renderer
- Electric blue theme easy to implement
**Alternatives**: Qt (heavy, licensing), Flutter (Dart), Native Android Views (JNI complexity)

## D008: AI Adapters → Adapter Pattern with HTTP
**Date**: 2026-11-15
**Decision**: Implement adapter interface, HTTP-based providers
**Rationale**:
- Supports any OpenAI-compatible API
- Ollama local support
- Claude Code via proxy/adapter
- No vendor lock-in
- Streaming via SSE
**Alternatives**: SDK per provider (bloat), Single provider (inflexible)

## D009: Project Format → JSON (.mbproj)
**Date**: 2026-11-15
**Decision**: Simple JSON for project files
**Rationale**:
- Human readable, version controllable
- Easy to parse in C++ (nlohmann/json) and Lua
- Extensible
- No binary format issues
**Alternatives**: XML (verbose), YAML (extra dep), SQLite (overkill)

## D010: Roblox Offsets → Dynamic Resolution
**Date**: 2026-11-15
**Decision**: Replace hardcoded offsets with pattern scanning + version detection
**Rationale**:
- Offsets change every Roblox update
- Pattern scanning finds functions by signature
- Version detection enables offset tables per version
- Survives updates without rebuild
**Alternatives**: Keep hardcoded (breaks constantly), User-provided (bad UX)

---

## Decision Template

### D### - Title
**Date**: YYYY-MM-DD
**Decision**: What was decided
**Rationale**: Why
**Alternatives Considered**: Other options
**Impact**: Affected components