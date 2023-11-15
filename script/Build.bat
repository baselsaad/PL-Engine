@echo off
pushd ..\
call script\premake\premake5.exe vs2022
popd
PAUSE