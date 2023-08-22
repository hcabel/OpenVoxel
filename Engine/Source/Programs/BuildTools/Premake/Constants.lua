include "../Constants.lua"

local outputDirPath = ROOT_DIR_PATH .. "build/"
local intermediateDirPath = ROOT_DIR_PATH .. "intermediate/"

OUTPUT_DIR_FORMAT = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/"

BUILD_OUTPUT_PATH = outputDirPath .. OUTPUT_DIR_FORMAT
INTERMEDIATE_OUTPUT_PATH = intermediateDirPath .. OUTPUT_DIR_FORMAT

PROJECT_FILE_OUTPUT_PATH = intermediateDirPath .. "ProjectFile/"

-- Output all the DLL exe and other in the same directory
-- (It's already a config specific folder anyway)
MODULE_OUTPUT_PATH = BUILD_OUTPUT_PATH
PROJECT_OUTPUT_DIR = BUILD_OUTPUT_PATH