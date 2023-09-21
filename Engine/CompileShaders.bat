@echo off

set "SHADER_PATH=Source/Runtime/Renderer/private/Shaders"
set "SHADER_TARGET_PATH=intermediate/Shaders"

@echo on

if not exist "%SHADER_TARGET_PATH%" mkdir "%SHADER_TARGET_PATH%"

call ..\Binairies\glslc.exe -g --target-env=vulkan1.3 %SHADER_PATH%/raytrace.rgen -o %SHADER_TARGET_PATH%/RayGen.spv
call ..\Binairies\glslc.exe -g --target-env=vulkan1.3 %SHADER_PATH%/raytrace.rmiss -o %SHADER_TARGET_PATH%/Miss.spv
call ..\Binairies\glslc.exe -g --target-env=vulkan1.3 %SHADER_PATH%/raytrace.rchit -o %SHADER_TARGET_PATH%/Hit.spv
call ..\Binairies\glslc.exe -g --target-env=vulkan1.3 %SHADER_PATH%/raytrace.rint -o %SHADER_TARGET_PATH%/Intersection.spv

pause