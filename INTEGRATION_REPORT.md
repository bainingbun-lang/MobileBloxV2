# Informe de integración — MobileBloxV2 + ExternalExecutor

Fecha: 2026-11-16
Repositorio: `bainingbun-lang/MobileBloxV2` (rama `master`)
Versión: `2.1.0` (versionCode 3)
Commit de esta iteración: `0bae8d6`
CI: run `43955417790` (verde, 12/12 pruebas)

## 1. Resumen

Se analizó [ExternalExecutor](https://github.com/ChimeraLle-Real/ExternalExecutor) (MIT, Copyright (c) 2025 ChimeraLle-Real) y se integró en MobileBloxV2 todo lo que es portable a Android/ARM64 sin romper la funcionalidad existente.

Además se añadió lo que pediste para el flujo tipo launcher:

- Abrir Roblox desde la app.
- Minimizar la app en picture-in-picture (PiP).
- Burbuja flotante móvil sobre cualquier app (incluido Roblox); al tocarla se abre un executor centrado con editor y salida.
- Permisos privilegiados vía **Shizuku** (shell ADB): conceder overlay, notificaciones y ejecutar comandos.
- **Compilador y runtime Luau nativo** (arm64-v8a) para ejecutar scripts de verdad dentro de la app.

### Límite importante (léelo)

El executor **no inyecta scripts en Roblox**. Los scripts se ejecutan en el runtime Luau propio de la app; no leen ni escriben la memoria de Roblox, no lo modifican y no se conectan a su proceso. Shizuku solo da permisos de shell/ADB, no acceso a la memoria privada de otras apps. Un "cliente modificado" requeriría root + ingeniería inversa del APK de Roblox y rompería la firma/ToS; no se implementó.

## 2. Fuente analizada

Ruta local: `/data/data/com.termux/files/usr/tmp/opencode/ExternalExecutor` (commit `3b6362f`).

| Archivo | Rol en el original | Estado en Android |
|---|---|---|
| `ExternalExecutor/Bridge.hpp` | HTTP `127.0.0.1:9611`, `/handle`, token | Portado (protocolo compatible) |
| `ExternalExecutor/Utils/Bytecode.hpp` | Codificador de bytecode Luau | Sustituido por el compilador Luau nativo |
| `ExternalExecutor/Instance.hpp` | Escritura de instancias en memoria | No portado (Win32) |
| `Memory.hpp`, `Process.hpp`, `Offsets.hpp` | Memoria/offsets del proceso | No portado (Win32/x64) |
| `ExternalExecutor/Init.lua` | Script de inicialización | No aplica sin inyección |
| `ExternalExecutorUI/Form1.cs` | UI WinForms | Sustituida por UI Android + burbuja flotante |

## 3. Qué se integró

| Capacidad | Implementación | Estado |
|---|---|---|
| Bridge local con token | `bridge/BridgeServer.kt` | Funcional |
| Protocolo `tipo\nclientId\njson\npayload` | `bridge/BridgeCommand.kt` | Funcional |
| `ping`, `identify` | `bridge/BridgeDispatcher.kt` | Funcional |
| `request` (proxy HTTP) | `bridge/HttpProxy.kt` (OkHttp) | Funcional |
| `readfile`, `writefile`, `listfiles`, `exists` | `bridge/FileRepository.kt` (sandbox) | Funcional |
| `listen` (cola de scripts) | `bridge/ScriptQueue.kt` | Funcional |
| `compile` | `luau/LuauBridge.kt` + JNI Luau | Funcional |
| `run` | Runtime Luau in-app | Funcional |
| `setscriptbytecode` | Carga bytecode Luau y lo ejecuta | Funcional |
| `shell` | `shizuku/ShizukuManager.kt` (shell Shizuku) | Funcional con Shizuku |
| `grant`, `grantoverlay` | `pm grant` / `appops` vía Shizuku | Funcional con Shizuku |
| `shizuku` | Estado del binder/permiso | Funcional |
| `launchroblox` | `launcher/RobloxLauncher.kt` | Funcional |
| Editor + ejecución local | `MainFragment.kt` | Funcional |
| Abrir Roblox + PiP | `MainActivity.kt` | Funcional (PiP según dispositivo) |
| Burbuja flotante + panel centrado | `overlay/OverlayService.kt` + `overlay_executor.xml` | Funcional con permiso overlay |
| Pruebas JVM | 12 pruebas en `app/src/test/.../bridge/` | Pasando |

## 4. Shizuku

La app usa `dev.rikka.shizuku:api:13.1.5` y `provider:13.1.5`.

Requisitos:

1. Instalar la app **Shizuku** (Play Store / GitHub) y activarla por ADB o root.
2. Abrir MobileBlox V2 → **Autorizar Shizuku** → aceptar.
3. Pulsar **Permiso overlay** (ejecuta `appops set com.mobileblox.v2 SYSTEM_ALERT_WINDOW allow`).
4. Pulsar **Burbuja flotante**; la burbuja queda sobre Roblox, se puede arrastrar y al tocarla se abre el executor centrado.

Comandos disponibles por el bridge (si Shizuku está autorizado):

- `shell` → ejecuta `sh -c <payload>` como UID shell y devuelve `code/stdout/stderr`.
- `grant` → `pm grant` para una lista blanca de permisos.
- `grantoverlay` → concede `SYSTEM_ALERT_WINDOW` por appops.
- `shizuku` → `{available, permission}`.
- `launchroblox` → abre el cliente Roblox instalado.

## 5. Luau nativo

- Fuente: `https://github.com/luau-lang/luau` commit `c87565b8b0db5d5d5bfd2b8b00ad6573e5d06d9f`.
- Se clona en CI con `scripts/fetch-luau.sh` (carpeta ignorada por git).
- `app/src/main/cpp/CMakeLists.txt` compila `Luau.Compiler`, `Luau.VM`, `Luau.Ast` y el wrapper `luau_jni.cpp` como `libmobileblox_luau.so`.
- Funciones JNI: `nativeCompile`, `nativeRun`, `nativeRunBytecode`.
- `print` se captura en `__output` y se devuelve como salida del script.
- Verificado en dispositivo ARM64: compila y ejecuta `print('hola', 2+2)` → `hola\t4`.

El APK contiene `lib/arm64-v8a/libmobileblox_luau.so` (≈1.2 MB) y `libc++_shared.so`.

## 6. Flujo de uso

1. Instalar `MobileBloxV2-ARM64.apk`.
2. (Opcional) Autorizar Shizuku y conceder overlay.
3. Pulsar **Burbuja flotante** para tener el acceso flotante.
4. Pulsar **Abrir Roblox** (o abrir Roblox manualmente).
5. Tocar la burbuja → se abre el executor flotante centrado (editor + salida + botones Execute/Limpiar/Cerrar). La burbuja se arrastra para moverla.
6. Escribir un script Luau y pulsar **Execute**: se ejecuta en el runtime de la app y la salida aparece en el panel. También se encola para clientes del bridge (`listen`).

## 7. Seguridad

- El bridge escucha solo en `127.0.0.1` y exige `X-Bridge-Token`.
- Archivos confinados a `filesDir/MobileBlox` (canonical path + rechazo de `..`, límite 5 MB).
- Shizuku se usa solo para permisos/overlay/shell; no hay evasión de anticheat ni ocultamiento.
- No se modifica ni redistribuye el APK de Roblox.

## 8. Pruebas y CI

Workflow: `.github/workflows/build-arm64.yml`

- JDK 17, Android SDK 34, NDK `26.1.10909125`, CMake `3.22.1`.
- `scripts/fetch-luau.sh` → `assembleRelease` → `testReleaseUnitTest` → verificación/subida del APK.
- Pruebas: `BridgeCommandParserTest` (3), `ScriptQueueTest` (4), `FileRepositoryTest` (3), `HttpProxyTest` (2) → **12/12**.
- Run verde: `43955417790`.

## 9. Artefactos

- APK: `artifacts/app-release.apk` (7.0 MB)
- Copia para instalar: `/sdcard/Download/MobileBloxV2-ARM64.apk`
- Backup previo: `/sdcard/Download/MobileBloxV2-backup-20261116_203859.tar.gz`
- Firmado con clave de debug (no hay `keystore.jks` en el repo).

## 10. Limitaciones y próximos pasos

- **Sin inyección en Roblox**: los scripts no afectan al juego. Es una limitación técnica de Android/Shizuku, no un bug.
- PiP y overlay dependen del fabricante; en algunos dispositivos PiP puede estar deshabilitado.
- El servicio de burbuja es `foregroundServiceType="specialUse"`; requiere notificación (Android 13+).
- Si se quiere un ejecutor real dentro de Roblox haría falta root + un inyector nativo específico por versión de Roblox; no es viable en este entorno.
- Próximo paso útil: añadir una librería de APIs simuladas (`game`, `workspace`, etc.) al runtime Luau para que los scripts típicos de Roblox no fallen al ejecutarse localmente.
