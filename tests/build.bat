@echo off

SET CompileOptions=/nologo /W3 /WX /wd4101 /wd4146 /wd4319 /wd4700 /wd4800 /wd4819
SET DebugBuild=/Zi /DDEBUG_BUILD

if not exist "..\build" mkdir "..\build"
pushd ..\build
call cl ..\tests\test-memory.c %CompileOptions% /I ..\src\ %DebugBuild% /link /INCREMENTAL:NO
call cl ..\tests\test-strings.c %CompileOptions% /I ..\src\ %DebugBuild% /link /INCREMENTAL:NO
call cl ..\tests\test-platform.cpp %CompileOptions% /EHa /I ..\src\ %DebugBuild% /link /INCREMENTAL:NO
popd