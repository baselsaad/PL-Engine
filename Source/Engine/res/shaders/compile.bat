@echo off

set VULKAN_SDK=%VULKAN_SDK%

:: Assuming the script is in the same directory as the shader files
set SCRIPT_DIR=%~dp0

:: Compile vertex shader
"%VULKAN_SDK%\Bin\glslc.exe" "%SCRIPT_DIR%shader.vert" -o "%SCRIPT_DIR%vert.spv"

:: Compile fragment shader
"%VULKAN_SDK%\Bin\glslc.exe" "%SCRIPT_DIR%shader.frag" -o "%SCRIPT_DIR%frag.spv"

pause