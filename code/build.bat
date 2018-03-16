@echo off

rem --------------------------------------------------------------------------
rem                        COMPILATION
rem --------------------------------------------------------------------------

set STBPath=..\..\stb\
set RivtenPath=..\..\rivten\

set UntreatedWarnings=/wd4100 /wd4244 /wd4201 /wd4127 /wd4505 /wd4456 /wd4996 /wd4003 /wd4706
set CommonCompilerDebugFlags=/MT /Od /Oi /fp:fast /fp:except- /Zo /Gm- /GR- /EHa /WX /W4 %UntreatedWarnings% /Z7 /nologo /I %STBPath% /I %RivtenPath%
set CommonLinkerDebugFlags=/incremental:no /opt:ref /subsystem:console /ignore:4099

pushd ..\build\
cl %CommonCompilerDebugFlags% ..\code\ion.cpp /link %CommonLinkerDebugFlags%
popd

rem --------------------------------------------------------------------------
echo Compilation completed...
