# MobileBloxV2 - Token Budget & Context Rules

## Token Saving Rules (Enforced)

### DO NOT
- ❌ Explain what you're about to do before doing it
- ❌ Repeat information already in PROJECT_INDEX.md, ARCHITECTURE.md, etc.
- ❌ Show full file contents when a summary suffices
- ❌ Describe every command before executing
- ❌ Produce verbose progress reports
- ❌ Re-analyze files already documented
- ❌ Generate documentation that duplicates existing docs

### DO
- ✅ Communicate only: changes made, errors found, what's missing, test results
- ✅ Use compact report format: [PHASE], [STATUS], [CHANGES], [TESTS], [PENDING]
- ✅ Update state files (WORKLOG, TODO, ERRORS, DECISIONS) incrementally
- ✅ Group related commands
- ✅ Run specific tests before general tests
- ✅ Save logs to files, read only relevant lines

## Context Management

### When Context Low
1. Stop work safely
2. Commit all changes
3. Update WORKLOG.md (last 5 actions)
4. Update TODO.md (current task + next 3)
5. Write technical summary (≤30 lines)
6. Identify exact next step
7. Resume using state files only

### Persistent State Files (Always Current)
- `WORKLOG.md` - Chronological action log
- `TODO.md` - Task tracker with checkboxes
- `ERRORS.md` - Known issues + solutions
- `DECISIONS.md` - Architectural decisions
- `PROJECT_INDEX.md` - Structure reference
- `ARCHITECTURE.md` - System design
- `DEPENDENCIES.md` - Dep versions + build plan
- `CHANGELOG.md` - Version history

## Report Format (Mandatory)

```
[PHASE]
Phase name

[STATUS]
Completada / En progreso / Bloqueada

[CAMBIOS]
file1: change summary
file2: change summary

[TESTS]
Pasaron / Fallaron / Pendientes

[PENDIENTE]
Next concrete task
```

## Current Phase
**FASE 2: ARM64 Compatibility** - CMake migration + Gradle setup

## Next Action
Create root CMakeLists.txt and NativeLib/CMakeLists.txt with Android toolchain configuration for arm64-v8a.