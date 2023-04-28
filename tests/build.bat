@echo off

SET CompileOpts=/nologo /I ..\src\ /Zi /DDEBUG_BUILD /W3 /WX /wd4101 /wd4146 /wd4319 /wd4700 /wd4800 /wd4819
SET LinkOpts=/link /INCREMENTAL:NO

if not exist "..\build" mkdir "..\build"
pushd ..\build
call cl ..\tests\test-memory.c %CompileOpts% %LinkOpts%
call cl ..\tests\test-strings.c %CompileOpts% %LinkOpts%
call cl ..\tests\test-platform.cpp %CompileOpts% /EHa %LinkOpts%
call cl ..\tests\add.c /LD /Zi %LinkOpts% /DLL /EXPORT:AddTwo
popd