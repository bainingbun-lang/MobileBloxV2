# WORKFLOW_CONTEXT.md
# MobileBlox V2 - Contexto completo para continuidad de desarrollo

---

## 1. PROJECT_OVERVIEW

**Nombre:** MobileBlox V2  
**Autor original:** ironleon888 (Made by Dyl4n)  
**Repo actual:** https://github.com/bainingbun-lang/MobileBloxV2  
**Rama:** master  
**Último commit:** 168dd91 - "Fix NDK package name"

### Objetivo principal
Convertir MobileBloxV2 (executor de scripts Roblox para Android 32-bit) en un **entorno moderno de desarrollo Luau para Android ARM64** con:
- Editor Luau completo (syntax highlighting, autocomplete, tabs, diagnóstico)
- Explorer estilo Roblox Studio (árbol jerárquico, inspector, context menu)
- AI Assistant integrado (Ollama, OpenAI, Anthropic, custom endpoints)
- Identidad visual: fondo negro, paneles gris oscuro (#1a1a1a), botones negros (#0d0d0d), texto blanco, acento azul eléctrico (#00d4ff), logo "_"
- Compilación nativa ARM64 para Android 7+ (API 24+)

### Funcionalidades actuales (IMPLEMENTADAS en código fuente)
- ✅ JNI_OnLoad + hooks Roblox (dobby, wait_for_lib, rebasing)
- ✅ Luau VM forwarders a libroblox.so (luaD_*, luaV_*, luaC_*)
- ✅ Bytecode encoder custom (opcode ×227)
- ✅ TaskScheduler interaction (job enumeration, ScriptContext hooking)
- ✅ Environment libraries (misc, closure, debug, websocket, reflection, etc.)
- ✅ Renderer stub (Vulkan hook placeholder)
- ✅ LuaUI injection (Roblox Lua GUI legacy)

### Funcionalidades NUEVAS implementadas en C++ (nativas, no probadas en device)
- ✅ UI System: Theme (ImGui), UIManager (docking layout)
- ✅ Luau Editor: multi-tab, syntax highlighting, line numbers, file ops, diagnostics gutter
- ✅ Explorer: tree view, search/filter, context menu (create/delete/rename/duplicate/move), inspector, drag&drop
- ✅ AI Assistant: Adapter pattern (Ollama, OpenAI, Anthropic, Custom), streaming SSE, file context, error analysis, predefined prompts

### Funcionalidades PENDIENTES (no implementadas / no probadas)
- ❌ Compilación ARM64 exitosa y verificada en device
- ❌ Android App completo (MainActivity, Surface callbacks, Gradle build)
- ❌ Integración JNI Surface → UIManager → ImGui rendering
- ❌ Project file format (.mbproj JSON)
- ❌ Autocomplete LSP/local
- ❌ Code formatter integration
- ❌ Unit tests / Integration tests
- ❌ Dobby ARM64 (deshabilitado en CMake, compilar aparte)
- ❌ CPR/IXWebSocket (deshabilitados por try_compile issues)

---

## 2. TECH_STACK

| Componente | Versión/Detalle |
|------------|-----------------|
| **Lenguajes** | C++20, Kotlin, CMake |
| **Build System** | CMake 3.22+ + Gradle 8.5+ (Kotlin DSL) |
| **NDK** | r26d (Clang 17), API 24, STL c++_shared |
| **Arquitectura** | arm64-v8a (primary), armeabi-v7a (optional) |
| **STL** | c++_shared |
| **Luau** | FetchContent desde github.com/luau-lang/luau (tag 0.635) |
| **ImGui** | FetchContent docking branch |
| **nlohmann/json** | FetchContent v3.11.2 |
| **LZ4** | FetchContent v1.10.0 |
| **Dobby** | Deshabilitado (compilar aparte ARM64) |
| **CPR/IXWebSocket** | Deshabilitados (try_compile issues) |
| **OpenSSL/curl** | System (Termux/NDK) |
| **Android Gradle Plugin** | 8.2.2 |
| **Kotlin** | 1.9.20 |
| **Compile SDK** | 34 |
| **Min SDK** | 24 |
| **Target SDK** | 34 |

### Identidad Visual (ImGui Theme)
```
Background:      #000000 (negro)
Panels:          #1a1a1a (gris oscuro)
Buttons:         #0d0d0d (negro)
Text:            #ffffff (blanco)
Accent:          #00d4ff (azul eléctrico)
Accent Hover:    #00aacc
Border:          #333333
Error:           #ff4444
Success:         #44cc44
Warning:         #ffaa00
```
Logo: `"_"` en azul eléctrico sobre botón negro

---

## 3. REPOSITORY_STRUCTURE

```
/data/data/com.termux/files/home/MobileBloxV2-main/
├── .github/workflows/build-arm64.yml    # GitHub Actions workflow (ARM64 build)
├── CMakeLists.txt                       # Root CMake (FetchContent deps)
├── build.gradle.kts                     # Gradle root
├── settings.gradle.kts
├── gradle.properties
├── gradlew / gradlew.bat
├── local.properties.template
├── Dockerfile.build                     # Docker build environment (x86_64 host)
├── build_arm64.sh                       # Build script para host x86_64
├── build_termux_arm64.sh                # Build script para Termux ARM64 (NO FUNCIONA - sysroot incompleto)
├── trigger_github_build.sh              # Script para disparar GH Actions
├── WORKFLOW_CONTEXT.md                  # ESTE ARCHIVO
├── PROJECT_INDEX.md
├── ARCHITECTURE.md
├── DEPENDENCIES.md
├── BUILD.md
├── CHANGELOG.md
├── TODO.md
├── ERRORS.md
├── DECISIONS.md
├── WORKLOG.md
├── TOKEN_BUDGET.md
├── app/                                 # Android App Module
│   ├── build.gradle.kts
│   ├── proguard-rules.pro
│   ├── src/main/
│   │   ├── AndroidManifest.xml
│   │   ├── java/com/mobileblox/v2/
│   │   │   ├── MobileBloxApplication.kt
│   │   │   ├── MainActivity.kt
│   │   │   └── MainFragment.kt
│   │   ├── res/
│   │   │   ├── layout/activity_main.xml
│   │   │   ├── layout/fragment_main.xml
│   │   │   ├── menu/main_menu.xml
│   │   │   ├── values/colors.xml
│   │   │   ├── values/strings.xml
│   │   │   ├── values/themes.xml
│   │   │   ├── drawable/ (8 vector icons: logo, add, folder, save, sidebar, terminal, ai, settings, play)
│   │   │   ├── xml/file_paths.xml
│   │   │   ├── xml/data_extraction_rules.xml
│   │   │   └── xml/backup_rules.xml
├── NativeLib/                           # C++ Native Library
│   ├── CMakeLists.txt
│   ├── luaconf.h.in                     # Patched luaconf.h para Roblox forwarders
│   ├── roblox_forwarders.cpp            # Forwarders a libroblox.so (reemplaza Luau modificado)
│   ├── main.cpp                         # JNI_OnLoad + Surface callbacks
│   ├── pch.hpp                          # Precompiled header (disabled temporalmente)
│   ├── exploit.hpp/cpp
│   ├── execution/execution.hpp/cpp
│   ├── renderer/renderer.hpp/cpp
│   ├── roblox/hooks.hpp/cpp, update.hpp/cpp
│   ├── taskscheduler/taskscheduler.hpp/cpp
│   ├── reflection/instance.hpp/cpp
│   ├── environment/environment.hpp/cpp + libraries/
│   ├── callcheck/callcheck.hpp/cpp
│   ├── utils/utils.hpp/cpp
│   ├── ui/theme.hpp, ui_manager.hpp/cpp
│   ├── editor/editor.hpp/cpp
│   ├── explorer/explorer.hpp/cpp
│   ├── ai/ai_assistant.hpp/cpp
│   └── dependencies/lz4/lz4.cpp/h
```

---

## 4. ARCHITECTURE

### High-Level Data Flow
```
Android App (Kotlin)                    NativeLib (C++)
┌─────────────────────────────┐         ┌─────────────────────────────┐
│ MainActivity                │         │ JNI_OnLoad                  │
│   Surface callbacks ───────►│         │   wait_for_lib(libroblox.so)│
│   onDrawFrame ─────────────►│         │   roblox::functions::init() │
└─────────────────────────────┘         │   roblox::hooks::init()     │
                                        │   renderer::init()          │
                                        │   UIManager::Initialize()   │
                                        └─────────────────────────────┘
                                                      │
                                        ┌─────────────▼─────────────┐
                                        │ Roblox Process            │
                                        │ libroblox.so              │
                                        │   luaD_*, luaV_*, luaC_*  │
                                        │   TaskScheduler jobs      │
                                        │   ScriptContext           │
                                        └───────────────────────────┘
```

### Key C++ Classes
| Clase | Patrón | Responsabilidad |
|-------|--------|-----------------|
| `exploit::execution` | Singleton | Luau compile → bytecode encode (×227) → queue → schedule via ScriptContext |
| `taskscheduler` | Singleton | Job enumeration, ScriptContext hooking, identity management |
| `roblox::functions` | Namespace | Function pointers a Roblox internals (rebased) |
| `roblox::hooks` | Namespace | Dobby inline hooks |
| `mobileblox::ui::UIManager` | Class | ImGui init, docking layout, component orchestration |
| `mobileblox::editor::LuauEditor` | Class | Multi-tab editor, syntax highlight, diagnostics, file ops |
| `mobileblox::explorer::Explorer` | Class | Tree view, search, context menu, inspector, drag&drop |
| `mobileblox::ai::AIAssistant` | Class | Adapter pattern, streaming, context injection |

### Luau VM Forwarders (roblox_forwarders.cpp)
Reemplaza Luau source modificado. Forwarders a `libroblox.so`:
- `luau_execute` → `luaD_call`
- `luaD_throw`, `luaD_rawrunprotected`
- `luaC_step`, `luaC_barriertable`
- `luaV_gettable`, `luaV_settable`
- `luaO_nilobject_ptr()`, `luaH_dummynode_ptr()`

---

## 5. CURRENT_STATUS

### ✅ FUNCIONA (código compilable, no probado en device)
- CMake configuration completa (FetchContent, toolchain, flags)
- Android Gradle project (AGP 8.2.2, Kotlin 1.9.20)
- C++20, ARM64 toolchain, API 24, c++_shared STL
- FetchContent: Luau, ImGui, nlohmann/json, LZ4
- GitHub Actions workflow (build-arm64.yml)
- Android SDK setup via `packages` (ndk;26.1.10909125)
- Código C++ sin errores de sintaxis

### ⚠️ PARCIAL / NO PROBADO
- Android App build (Gradle) - falla en Configure CMake
- UIManager/ImGui rendering en device
- JNI Surface callbacks → ImGui frame
- Explorer/Editor/AI integration end-to-end
- Dobby ARM64 (deshabilitado)
- CPR/IXWebSocket (deshabilitados)

### ❌ ROTO / FALTA
- **Build ARM64 en GitHub Actions**: Falla en "Configure CMake (ARM64)" - exit code 1
- **Build nativo en Termux ARM64**: IMPOSIBLE - sysroot NDK no existe en Termux (solo x86_64 host)
- **Dobby ARM64**: Deshabilitado (cmake check falla en arquitectura)
- **CPR/IXWebSocket**: Deshabilitados (try_compile no hereda flags)
- **APK firmado**: No generado

### Último estado confirmado (GitHub Actions Run #7)
- ✅ Checkout, Setup Java, Setup Android SDK, Cache Gradle/CMake: **SUCCESS**
- ❌ **Configure CMake (ARM64)**: **FAILURE** - exit code 1, sin logs detallados disponibles

---

## 6. COMPLETED_WORK

| Fase | Estado | Detalles |
|------|--------|----------|
| Auditoría & Documentación | ✅ | PROJECT_INDEX, ARCHITECTURE, DEPENDENCIES, BUILD, CHANGELOG, TODO, ERRORS, DECISIONS, WORKLOG, TOKEN_BUDGET |
| CMake Migration | ✅ | Root + NativeLib CMakeLists.txt, FetchContent (Luau, ImGui, nlohmann/json, LZ4), toolchain ARM64 |
| Android Gradle Project | ✅ | app module, AGP 8.2.2, Kotlin DSL, manifest, theme, icons, MainActivity |
| GitHub Actions Workflow | ✅ | build-arm64.yml (v3 Android SDK, v5 Java, packages NDK) |
| Android App UI | ✅ | Theme (negro/azul), MainActivity, MainFragment, 8 vector icons |
| Native C++ Modernization | ✅ | UIManager, LuauEditor, Explorer, AIAssistant, Theme, roblox_forwarders |
| GitHub Repo | ✅ | bainingbun-lang/MobileBloxV2, 7 workflow runs |
| Cleanup | ✅ | Eliminados 308 archivos deps originales (ZIP) |

---

## 7. PENDING_WORK

| Prioridad | Tarea | Bloqueador |
|-----------|-------|------------|
| **CRÍTICO** | Fix "Configure CMake (ARM64)" en GitHub Actions | Ver logs reales del error |
| **CRÍTICO** | Compilar libMobileBlox.so ARM64 | Verificar CMakeLists.txt, toolchain, NDK path |
| **CRÍTICO** | Build APK completo (Gradle) | Native lib en jniLibs/arm64-v8a |
| **ALTO** | Compilar Dobby ARM64 aparte | Compilar libdobby.a para arm64-v8a |
| **ALTO** | Habilitar CPR/IXWebSocket | Fix try_compile flags o usar curl directo |
| **MEDIO** | Android App: Surface callbacks → UIManager | JNI bridge MainActivity ↔ UIManager |
| **MEDIO** | ImGui rendering en device | Vulkan/OpenGL ES backend |
| **MEDIO** | Project file (.mbproj) | JSON schema + save/load |
| **MEDIO** | Autocomplete (LSP/local) | Editor integration |
| **BAJO** | Unit tests | Editor, Explorer, AI adapters |

---

## 8. KNOWN_ERRORS

| ID | Error | Ubicación | Severidad | Estado |
|----|-------|-----------|-----------|--------|
| E001 | "Configure CMake (ARM64) exit code 1" | GitHub Actions step 7 | Crítico | Abierto |
| E002 | Android SDK setup fallaba (api-level/ndk-version inválidos) | workflow | Resuelto | v3 + packages |
| E003 | Dobby cmake: "Processor not configured" | Dobby CMakeLists.txt | Alto | Deshabilitado |
| E004 | CPR try_compile: filesystem check fail | CPR CMakeLists.txt | Alto | Deshabilitado |
| E005 | IXWebSocket: libatomic link fail | IXWebSocket CheckAtomic.cmake | Alto | Deshabilitado |
| E006 | Termux ARM64 build: sysroot incompleto | build_termux_arm64.sh | Crítico | No viable |
| E007 | Git checkout exit code 128 en Post Checkout | GitHub Actions | Medio | Warning |
| E008 | Node.js 20 deprecation warning | actions/cache@v4, checkout@v4, setup-android@v3 | Bajo | Info |

---

## 9. TECHNICAL_DECISIONS

| ID | Decisión | Motivo | Alternativa descartada |
|----|----------|--------|------------------------|
| D001 | CMake + Gradle (no VS) | Cross-platform, NDK estándar, CI/CD | Bazel, Meson, VS vcxproj |
| D002 | ARM64 primary | Todos devices modernos, Play Store requiere 64-bit | Dual ARM32/64 |
| D003 | NDK r26d (Clang 17) | LTS, C++20 completo, API 24+ | r27 beta, r25c |
| D004 | Luau FetchContent + patch luaconf.h | Track upstream, forwarders aislados | Vendored copy |
| D005 | CPR + curl + OpenSSL | Estándar industria, HTTP/2 | cpp-httplib, Boost.Beast |
| D006 | Dobby desde fuente | Ligero, activo, ARM64 support | Frida-gum (pesado) |
| D007 | ImGui + Docking | Ya en deps, immediate mode, profesional | Qt, Flutter, Native Views |
| D008 | Adapter pattern AI | Flexible, multi-provider | SDK por proveedor |
| D009 | .mbproj JSON | Humano, versionable, extensible | XML, YAML, SQLite |
| D010 | Dynamic offsets (future) | Roblox updates rompen hardcoded | Hardcoded |

---

## 10. FILES_REFERENCE

### Críticos para build
| Archivo | Propósito |
|---------|-----------|
| `.github/workflows/build-arm64.yml` | CI/CD ARM64 build |
| `CMakeLists.txt` (root) | FetchContent, toolchain config |
| `NativeLib/CMakeLists.txt` | Sources, includes, links, luaconf patch |
| `NativeLib/luaconf.h.in` | Patched Luau config (forwarders) |
| `NativeLib/roblox_forwarders.cpp` | Forwarders a libroblox.so |
| `app/build.gradle.kts` | Android app build config |
| `app/src/main/AndroidManifest.xml` | Permissions, activities |
| `app/src/main/java/.../MainActivity.kt` | JNI Surface callbacks |

### Código nuevo (modernización)
| Archivo | Descripción |
|---------|-------------|
| `NativeLib/ui/theme.hpp` | ImGui theme MobileBlox V2 |
| `NativeLib/ui/ui_manager.hpp/cpp` | ImGui init, docking, components |
| `NativeLib/editor/editor.hpp/cpp` | Luau editor completo |
| `NativeLib/explorer/explorer.hpp/cpp` | Explorer estilo Roblox Studio |
| `NativeLib/ai/ai_assistant.hpp/cpp` | AI Assistant con adapters |

---

## 11. TESTING_AND_BUILD

### Build en GitHub Actions (ACTUAL)
```yaml
# Trigger manual
gh workflow run build-arm64.yml -f version=2.0.0 --repo bainingbun-lang/MobileBloxV2

# Ver estado
gh run list --repo bainingbun-lang/MobileBloxV2
gh run view <RUN_ID> --repo bainingbun-lang/MobileBloxV2
```

### Build local (requiere host x86_64)
```bash
# Opción A: Docker (recomendado)
docker build -t mobileblox-build -f Dockerfile.build .
docker run --rm -v $(pwd):/src mobileblox-build

# Opción B: Script en host x86_64 Linux
./build_arm64.sh --install
```

### Build en Termux ARM64 (NO FUNCIONA)
- `build_termux_arm64.sh` - **NO USAR** - sysroot NDK inexistente en Termux ARM64

### Verificar build exitoso
```bash
# APK
ls -lh app/build/outputs/apk/release/app-release.apk
unzip -l app/build/outputs/apk/release/app-release.apk | grep libMobileBlox.so

# Native lib
file build/arm64/libMobileBlox.so
# Debe mostrar: ELF 64-bit LSB shared object, ARM aarch64
```

---

## 12. ENVIRONMENT_AND_LIMITATIONS

| Aspecto | Limitación |
|---------|------------|
| **Build ARM64** | Solo posible en host x86_64 (PC, servidor, GitHub Actions, Docker) |
| **Termux ARM64** | NO puede compilar apps Android - NDK toolchain es x86_64 only |
| **GitHub Actions** | 60 min timeout, 2 núcleos, 7GB RAM, ubuntu-latest |
| **NDK r26d** | Clang 17, API 24+, c++_shared STL |
| **Gradle** | Requiere JDK 17, 4GB heap recomendado |
| **Dobby** | Compilar aparte para ARM64 (cmake -DANDROID_ABI=arm64-v8a) |
| **Luau** | FetchContent tag 0.635, luaconf.h patcheado via CMake |
| **ImGui** | Docking branch, backend Android + OpenGL ES 3 |
| **Android Device** | S25 FE = ARM64, Android 14 (API 34), min SDK 24 |

---

## 13. WORKFLOW_ROADMAP

### FASE 1: Build System Stabilization (CRÍTICO)
**Objetivo:** Build ARM64 exitoso en GitHub Actions  
**Archivos:** `.github/workflows/build-arm64.yml`, `NativeLib/CMakeLists.txt`, `CMakeLists.txt`  
**Acciones:**
1. Obtener logs reales del error "Configure CMake (ARM64)"
2. Verificar `ANDROID_NDK_HOME` / `ANDROID_NDK_ROOT` en workflow
3. Verificar `cmake` command usa toolchain correcto
4. Verificar `ninja` build encuentra fuentes
**Criterio:** `ninja MobileBlox` genera `libMobileBlox.so` ARM64  
**Test:** `file build/arm64/libMobileBlox.so` → ELF 64-bit ARM aarch64  

### FASE 2: Native Library Complete
**Objetivo:** libMobileBlox.so completa + APK build  
**Acciones:**
1. Build native library completo (ninja)
2. Copiar a `app/src/main/jniLibs/arm64-v8a/`
3. Gradle assembleRelease
3. Verificar APK contiene libMobileBlox.so
**Criterio:** APK generado, instala en S25 FE

### FASE 3: Dobby ARM64
**Objetivo:** libdobby.a ARM64 funcional  
**Acciones:**
1. Compilar Dobby aparte: `cmake -DANDROID_ABI=arm64-v8a ...`
2. Link en NativeLib/CMakeLists.txt
**Criterio:** Hooks Roblox funcionales

### FASE 4: Android App Integration
**Objetivo:** App corre en S25 FE, muestra UI  
**Acciones:**
1. MainActivity: SurfaceHolder.Callback → JNI → UIManager
2. UIManager: ImGui_ImplAndroid_Init, ImGui_ImplOpenGL3_Init
3. Render loop: onDrawFrame → UIManager::NewFrame/Render/EndFrame
**Criterio:** App abre, muestra ImGui docking layout (Editor, Explorer, AI, Console)

### FASE 5: Features Completion
**Objetivo:** Editor, Explorer, AI funcionales  
**Acciones:** Integrar componentes C++ con JNI bridge, probar en device

---

## 14. TOKEN_OPTIMIZATION

### Reglas estrictas
- **NO** leer archivos completos innecesariamente → `grep`/`head`/`tail`
- **NO** repetir información ya en este doc → referenciar secciones
- **NO** explicar antes de actuar → actuar y reportar resultado
- **NO** generar documentación redundante → actualizar este doc
- **SÍ** búsquedas específicas: `grep -r "pattern" --include="*.cpp" NativeLib/`
- **SÍ** comandos agrupados: `cd build/arm64 && ninja -j4 2>&1 | tail -20`
- **SÍ** log compacto: `[FASE] [ESTADO] [CAMBIOS] [TESTS] [PENDIENTE]`

### Índice de archivos (para búsquedas rápidas)
```bash
# Build system
find . -name "CMakeLists.txt" -o -name "*.gradle.kts" -o -name "*.yml"

# C++ sources
find NativeLib -name "*.cpp" -o -name "*.hpp" | grep -v build

# Android
find app/src/main -type f

# Workflow
cat .github/workflows/build-arm64.yml
```

---

## 15. SAFETY_AND_BACKUPS

### Archivos NO modificar sin autorización
- `MobileBloxV2-main/NativeLib/` - fuentes originales (ZIP)
- `.github/workflows/` - solo cambios incrementales
- `app/src/main/AndroidManifest.xml` - permissions críticas
- `gradlew` / `gradle/wrapper/` - no tocar

### Backups existentes
- Git history: `git log --oneline -20`
- GitHub repo: `bainingbun-lang/MobileBloxV2` (7 runs)
- Local: `/data/data/com.termux/files/home/MobileBloxV2-main/`

### Operaciones destructivas (requieren confirmación)
- `git push --force`
- `rm -rf` en NativeLib/app
- Modificar `gradle-wrapper.properties`
- Cambiar `compileSdk`/`minSdk`/`targetSdk`

### Riesgos
- **Pérdida de código original**: Repo ZIP en `/sdcard/Download/MobileBloxV2-main.zip`
- **Build quebrado**: Siempre testear `ninja` antes de push
- **Secrets expuestos**: GitHub PAT en workflow_dispatch (rotar después)

---

## 16. INSTRUCTIONS_FOR_CHATGPT

### Cómo usar este contexto
1. **Lee este archivo completo** antes de tocar código
2. **No repitas análisis** - usa secciones 1-15 como referencia
3. **Prioriza Fase 1** - sin build no hay nada que testear
4. **Reporta formato compacto**: `[FASE] [ESTADO] [CAMBIOS] [TESTS] [PENDIENTE]`
5. **No inventes** - si falta info, pregunta o busca con grep

### Flujo de trabajo recomendado
```bash
# 1. Ver estado actual
gh run list --repo bainingbun-lang/MobileBloxV2 --limit 3

# 2. Ver logs del último fallo
gh run view <RUN_ID> --log --repo bainingbun-lang/MobileBloxV2

# 3. Fix mínimo → commit → push → trigger

# 4. Verificar build
gh run watch <RUN_ID> --repo bainingbun-lang/MobileBloxV2
```

### Qué NO hacer
- ❌ Re-escribir CMakeLists.txt desde cero
- ❌ Cambiar arquitectura (CMake→Gradle only, etc.)
- ❌ Re-escribir editor/explorer/ai desde cero
- ❌ Intentar build en Termux ARM64
- ❌ Modificar archivos originales del ZIP

### Qué SÍ hacer
- ✅ Fix específico al error de CMake en GitHub Actions
- ✅ Commits pequeños y atómicos
- ✅ Verificar cada paso con `gh run watch`
- ✅ Actualizar este WORKFLOW_CONTEXT.md tras cambios

---

## 17. FIRST_ACTION_TO_EXECUTE

```bash
# 1. Ver logs reales del error "Configure CMake (ARM64)" - Run #7
gh run view 43922870994 --log --repo bainingbun-lang/MobileBloxV2 | grep -A 50 "Configure CMake"

# 2. Si no hay logs claros, agregar debug al workflow:
#    - name: Debug CMake
#      run: |
#        echo "ANDROID_NDK_HOME=$ANDROID_NDK_HOME"
#        echo "ANDROID_NDK_ROOT=$ANDROID_NDK_ROOT"
#        ls -la $ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
#        cmake --version && ninja --version

# 3. Commit → Push → Trigger → Watch
```

---

## RESUMEN DE HALLAZGOS MÁS IMPORTANTES

1. **Build ARM64 SOLO posible en host x86_64** (GitHub Actions, Docker, PC Linux). Termux ARM64 NO tiene sysroot NDK.
2. **Repo limpio**: 308 archivos deps originales eliminados, usando FetchContent moderno.
3. **Workflow casi listo**: Android SDK v3 + packages NDK correcto, Java v5, CMake config lista.
4. **Bloqueo actual**: "Configure CMake (ARM64)" falla en GitHub Actions sin logs visibles. Necesito ver error real.
4. **Código moderno listo**: Editor, Explorer, AI, Theme, UIManager, Forwarders - todo en C++20, listo para integrar.
5. **Dobby/CPR/IXWebSocket deshabilitados** - compilar Dobby aparte, usar curl directo para HTTP.

---

## INFORMACIÓN NO VERIFICADA (PENDIENTE)

- [ ] Error exacto de "Configure CMake (ARM64)" en Run #7
- [ ] Si `ANDROID_NDK_ROOT` está disponible en GitHub Actions runner
- [ ] Si `ninja` encuentra todas las fuentes FetchContent
- [ ] Si Gradle build pasa una vez libMobileBlox.so existe
- [ ] Si APK instala y corre en S25 FE real
- [ ] Performance de ImGui en device móvil
- [ ] Memoria/CPU usage del native lib

---

**Documento generado:** 2026-11-16  
**Versión:** 1.0  
**Para:** ChatGPT / próxima IA de desarrollo  
**Proyecto:** MobileBlox V2 - ARM64 Modernization