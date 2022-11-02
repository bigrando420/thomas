@echo off

rd /s /q build
mkdir build

xcopy /y /q /E data build

pushd build
rem -DTH_SPEED=1 (removes asserts) TH_RELEASE
cl -Zi -FC -I..\sauce\ ../sauce/anvil.c -Fe:anvil.exe user32.lib telescope_core.lib
popd