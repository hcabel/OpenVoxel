VULKAN_SDK = os.getenv("VULKAN_SDK")

ROOT_DIR_PATH = os.getenv("PWD") or io.popen("cd"):read()
-- replace windows slashes with linux ones
ROOT_DIR_PATH = string.gsub(ROOT_DIR_PATH, '\\', '/')
-- Remove last part of the path
ROOT_DIR_PATH = string.gsub(ROOT_DIR_PATH, "Source/Programs/BuildTools", "")

local outputDirPath = ROOT_DIR_PATH .. "build/"
local intermediateDirPath = ROOT_DIR_PATH .. "intermediate/"

OUTPUT_DIR_FORMAT = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/"

BUILD_OUTPUT_PATH = outputDirPath .. OUTPUT_DIR_FORMAT
INTERMEDIATE_OUTPUT_PATH = intermediateDirPath .. OUTPUT_DIR_FORMAT
PROJECT_FILE_OUTPUT_PATH = intermediateDirPath .. "ProjectFile/"