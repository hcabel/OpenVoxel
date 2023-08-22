
ROOT_DIR_PATH = os.getenv("PWD") or io.popen("cd"):read()
-- replace windows slashes with linux ones
ROOT_DIR_PATH = string.gsub(ROOT_DIR_PATH, '\\', '/')
-- Remove last part of the path
ROOT_DIR_PATH = string.gsub(ROOT_DIR_PATH, "Source/Programs/BuildTools", "")