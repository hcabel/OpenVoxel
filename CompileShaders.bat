call vendor\bin\glslc.exe --target-env=vulkan1.3 Source/Runtime/Renderer/private/Shaders/raytrace.rgen -o intermediate/Shaders/RayGen.spv
call vendor\bin\glslc.exe --target-env=vulkan1.3 Source/Runtime/Renderer/private/Shaders/raytrace.rmiss -o intermediate/Shaders/Miss.spv
call vendor\bin\glslc.exe --target-env=vulkan1.3 Source/Runtime/Renderer/private/Shaders/raytrace.rchit -o intermediate/Shaders/Hit.spv
call vendor\bin\glslc.exe --target-env=vulkan1.3 Source/Runtime/Renderer/private/Shaders/raytrace.rint -o intermediate/Shaders/Intersection.spv
pause