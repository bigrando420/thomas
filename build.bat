@echo off

rem -DTH_SPEED=1 (removes asserts) TH_RELEASE

rd /s /q build
mkdir build
pushd build
cl -Zi /std:c++20 -FC -I..\sauce\ ../sauce/main.cpp -Fe:anvil.exe user32.lib
popd