VULKAN_SDK = os.getenv("VULKAN_SDK")

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

rootDir = os.getcwd()

buildDir = rootDir .. "/build"
intermediateDir = rootDir .. "/intermediate"

buildOutput = buildDir .. "/" .. outputdir
intermediateOutput = intermediateDir .. "/" .. outputdir

projectFileLocation = rootDir .. "/intermediate/ProjectFile"
