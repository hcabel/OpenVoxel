VULKAN_SDK = os.getenv("VULKAN_SDK")

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

rootDir = os.getcwd()

-- if rootDir end with "vendor/Premake5", remove it
-- this mean that the solution is generated with the build action
if string.find(rootDir, "vendor/Premake5") then
	rootDir = string.gsub(rootDir, "vendor/Premake5", "")
end

buildDir = rootDir .. "/build"
intermediateDir = rootDir .. "/intermediate"

buildOutput = buildDir .. "/" .. outputdir
intermediateOutput = intermediateDir .. "/" .. outputdir

projectFileLocation = rootDir .. "/intermediate/ProjectFile"
