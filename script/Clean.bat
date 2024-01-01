@echo off
pushd ..\
call Script\premake\premake5.exe --file=Build.lua clean
popd
PAUSE