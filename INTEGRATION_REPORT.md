# Informe de integración — MobileBloxV2 + ExternalExecutor

Fecha: 2026-11-16
Repositorio: `bainingbun-lang/MobileBloxV2` (rama `master`)
Commit de integración: `6dca9af` (corrección de cabeceras: `ba86f39`)

## 1. Resumen

Se analizó el proyecto [ExternalExecutor](https://github.com/ChimeraLle-Real/ExternalExecutor) (MIT, Copyright (c) 2025 ChimeraLle-Real) y se integraron en MobileBloxV2 las capacidades que son portables a Android/ARM64 **sin eliminar ni romper la funcionalidad existente**. El ejecutor original es una aplicación Windows x64 (Win32 + WinForms) que ataca un proceso Roblox de escritorio; su núcleo (memoria, hooks, bytecode Luau, UI WinForms) no es portable a Android y no se copió.

Lo integrado es la capa de automatización y comunicación: servidor HTTP local con el mismo protocolo del bridge, proxy HTTP, repositorio de archivos en sandbox, cola de scripts y editor en la UI. Todo compila, pasa pruebas y genera APK ARM64.

## 2. Fuente analizada

Ruta local: `/data/data/com.termux/files/usr/tmp/opencode/ExternalExecutor` (commit `3b6362f`).

Componentes revisados:

| Archivo | Rol en el original | Portable a Android |
|---|---|---|
| `ExternalExecutor/Bridge.hpp` | Servidor HTTP `127.0.0.1:9611`, ruta `/handle`, token | Sí (protocolo) |
| `ExternalExecutor/Utils/Bytecode.hpp` | Codificador de bytecode Luau (`opcode * 227`) | No (requiere bytecode Luau nativo) |
| `ExternalExecutor/Instance.hpp` | Escritura de instancias en memoria del proceso | No (Win32) |
| `ExternalExecutor/Memory.hpp`, `Process.hpp`, `Offsets.hpp` | Lectura/escritura de memoria y offsets de Roblox | No (Win32/x64) |
| `ExternalExecutor/Init.lua` | Script Luau de inicialización del lado del juego | No aplica sin inyección |
| `ExternalExecutorUI/Form1.cs` | UI WinForms | No (reemplazada por UI Android) |

## 3. Qué se integró

| Capacidad | Implementación Android | Estado |
|---|---|---|
| Servidor bridge local | `BridgeServer.kt` (NanoHTTPD en `127.0.0.1:9611`, `POST /handle`) | Funcional |
| Autenticación por token | `BridgeTokenStore.kt` + cabecera `X-Bridge-Token` | Funcional |
| Parseo del protocolo | `BridgeCommand.kt` (`tipo\nclientId\njson\npayload`) | Funcional |
| Comandos `ping`, `identify` | `BridgeDispatcher.kt` | Funcional |
| Comandos `request` (HTTP) | `HttpProxy.kt` (OkHttp, JSON `b,c,r,v,h`) | Funcional |
| `readfile`, `writefile`, `listfiles`, `exists` | `FileRepository.kt` (sandbox en `filesDir/MobileBlox`) | Funcional |
| `listen` (polling de scripts) | `ScriptQueue.kt` | Funcional |
| `compile`, `setscriptbytecode` | Responden `501 Not Implemented` | Pendiente (núcleo nativo) |
| Editor de scripts + cola | `MainFragment.kt` + `fragment_main.xml` | Funcional |
| Ciclo de vida del bridge | `BridgeService.kt` + `MobileBloxApplication.kt` | Funcional |
| Pruebas unitarias JVM | 12 pruebas en `app/src/test/.../bridge/` | Pasando |

## 4. Protocolo del bridge

```
POST http://127.0.0.1:9611/handle
X-Bridge-Token: <token generado>

<cuerpo>
tipo
clientId
{json de settings}
payload (resto, puede contener saltos de línea)
```

Respuesta: JSON. El token se genera una vez y se guarda en `SharedPreferences` (`bridge_prefs`). La UI muestra la URL y el token en `bridge_info`.

Comandos de escritura devuelven `{ "ok": true, ... }`; errores devuelven `{ "ok": false, "error": "..." }` con código HTTP 400/401/404/500.

## 5. Seguridad

- El servidor escucha **solo** en `127.0.0.1` (no expuesto a la red).
- Token aleatorio obligatorio en cada petición.
- Todas las rutas de archivo se resuelven con `File.getCanonicalFile()` y deben quedar dentro del sandbox; se rechaza cualquier intento de `..`.
- Límite de escritura: 5 MB por archivo.
- Sin evasión de anticheat, sin ocultamiento, sin manipulación de otros procesos.

## 6. Qué no se portó y por qué

- **Memoria/offsets/hooks de Roblox de escritorio**: dependen de Win32 (`OpenProcess`, `ReadProcessMemory`, `WriteProcessMemory`) y de offsets x64 de una versión concreta de Roblox. No aplican a Android ARM64.
- **Codificador de bytecode Luau**: `Bytecode.hpp` transforma bytecode ya compilado (`opcode * 227`). Requiere un compilador Luau nativo y el runtime del juego; no hay sustituto Kotlin equivalente. Los comandos quedan como `501` hasta integrar un compilador nativo.
- **`Init.lua` / inyección**: requiere un mecanismo de ejecución dentro del cliente, que no existe en este proyecto.
- **UI WinForms**: sustituida por la UI Android existente.

La carpeta `NativeLib/` sigue sin compilarse: su `CMakeLists.txt` referencia fuentes inexistentes (`../MobileBloxV2-main/NativeLib/...`). El código tolera la ausencia de la librería con `UnsatisfiedLinkError` controlado.

## 7. Pruebas y CI

Workflow: `.github/workflows/build-arm64.yml`

Pasos: checkout → JDK 17 → Android SDK (`platforms;android-34`, `build-tools;34.0.0`) → `assembleRelease` → `testReleaseUnitTest` → verificación de APK → subida de artefactos → release en tag.

Pruebas:

- `BridgeCommandParserTest` (3)
- `ScriptQueueTest` (4)
- `FileRepositoryTest` (3, incluye rechazo de path traversal)
- `HttpProxyTest` (2, con MockWebServer)

Resultado: **12/12 pruebas pasando**. Run de CI: `43952876951`.

## 8. Artefactos

- APK: `artifacts/app-release.apk` (6.1 MB)
- Copia para instalar: `/sdcard/Download/MobileBloxV2-ARM64.apk`
- Resultados de pruebas: artefacto `test-results` del run de CI
- Backup previo a la integración: `/sdcard/Download/MobileBloxV2-backup-20261116_203859.tar.gz`

El APK está firmado con la clave de debug (no hay `keystore.jks` en el repo). Para distribución hay que añadir un keystore de release.

## 9. Uso

1. Instalar `MobileBloxV2-ARM64.apk`.
2. Abrir la app: el bridge arranca automáticamente y muestra URL y token.
3. Enviar peticiones desde otro proceso/dispositivo en el mismo host:

```bash
curl -s http://127.0.0.1:9611/handle \
  -H "X-Bridge-Token: <token>" \
  --data-binary $'ping\ncli-1\n{}\n'
```

4. Escribir un script en el editor y pulsar **Ejecutar**: se encola y se entrega al siguiente `listen`.
5. **Guardar** / **Abrir** usan `filesDir/MobileBlox/scripts/main.lua`.

## 10. Limitaciones y próximos pasos

- `compile` y `setscriptbytecode` sin implementar (requieren núcleo nativo Luau).
- El bridge no está ligado a un servicio en primer plano; el sistema puede matar el proceso en segundo plano.
- Sin autenticación más allá del token; no exponer el puerto fuera de localhost.
- Próximo paso natural: integrar un compilador Luau nativo (por ejemplo, `luau-compile`) como librería JNI y conectar `compile`/`setscriptbytecode`.
