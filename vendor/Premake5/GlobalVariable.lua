VULKAN_SDK = os.getenv("VULKAN_SDK")

OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

RootDir = os.getcwd()

-- if RootDir end with "vendor/Premake5", remove it
-- this mean that the solution is generated with the build action
if string.find(RootDir, "/vendor/Premake5") then
	RootDir = string.gsub(RootDir, "/vendor/Premake5", "")
end

BuildDir = RootDir .. "/build"
IntermediateDir = RootDir .. "/intermediate"

BuildOutput = BuildDir .. "/" .. OutputDir
IntermediateOutput = IntermediateDir .. "/" .. OutputDir

ProjectFileLocationOutput = RootDir .. "/intermediate/ProjectFile"

