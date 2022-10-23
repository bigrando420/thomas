@echo off

rd /s /q build
mkdir build

xcopy /y /q /E data build

pushd build
rem -DTH_SPEED=1 (removes asserts) TH_RELEASE
cl -Zi /std:c++20 -FC -I..\sauce\ ../sauce/anvil.cpp -Fe:anvil.exe user32.lib
popd