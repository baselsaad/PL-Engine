@echo off
pushd ..\
call Script\premake\premake5.exe --file=Build.lua vs2022
popd
PAUSE