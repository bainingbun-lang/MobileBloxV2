# MobileBloxV2 - Errors & Solutions

## Known Issues (Pre-Modernization)

| ID | Error | Location | Severity | Status |
|----|-------|----------|----------|--------|
| E001 | Hardcoded offsets break on Roblox update | `update.hpp` | Critical | Open |
| E002 | ARM32 only - no arm64-v8a support | `vcxproj`, prebuilt libs | Critical | Open |
| E003 | Clang 5.0 outdated (2017) | `vcxproj` | High | Open |
| E004 | API 21 too low (Android 5.0) | `vcxproj` | Medium | Open |
| E005 | Race conditions on script queues | `execution.hpp/cpp` | High | Open |
| E006 | No mutex protection on singleton access | Multiple | Medium | Open |
| E007 | Prebuilt .a files - cannot update/debug | `dependencies/*/armeabi-v7a/` | High | Open |
| E008 | No Android app module - cannot run standalone | Missing | Critical | Open |
| E009 | ImGui included but not used for UI | `LuaUI.hpp` | Low | Open |
| E010 | No crash recovery / graceful degradation | `main.cpp` | High | Open |
| E011 | SELinux may block memory operations | `utils/memory.cpp` | High | Open |
| E012 | STL c++_shared conflict risk | `vcxproj` | Medium | Open |
| E013 | Bytecode encoder (×227) may not match new Luau | `execution.hpp` | Medium | Open |
| E014 | No version detection for Roblox | `update.hpp` | High | Open |
| E015 | Dobby ARM32 only | `dependencies/dobby/armeabi-v7a/` | Critical | Open |

## Template for New Errors

### E### - Brief Description
**Location**: `file.cpp:line`
**Error**: Exact error message
**Cause**: Root cause analysis
**Solution**: Fix applied
**Status**: Fixed / Workaround / Open
**Date**: YYYY-MM-DD