@echo off

rd /s /q build
mkdir build

pushd build
cl /Zi /nologo /FC /I..\sauce\third_party\telescope\ /I..\sauce\ /DCORE_BUILD=1 ..\sauce\third_party\telescope_light.c /link /DLL user32.lib gdi32.lib dwmapi.lib UxTheme.lib winmm.lib Advapi32.lib shell32.lib ole32.lib /out:telescope_core.dll
xcopy /y /q telescope_core.dll ..\data\
xcopy /y /q telescope_core.lib ..\data\
popd