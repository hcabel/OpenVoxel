#include "Windows/WindowsPlatformFileSystem.h"

#include <Windows.h>

std::string WindowsPlatformFileSystem::MakeEngineRootDirectoryPath()
{
	// Get binary path
	char buffer[MAX_PATH];
	if (GetModuleFileNameA(NULL, buffer, MAX_PATH) == 0)
		return ("");
	std::string binaryPath = std::string(buffer);

	// Remove everything after "build/"
	size_t posOfBuildFolder = binaryPath.rfind("build\\");
	if (posOfBuildFolder != std::string::npos)
		binaryPath = binaryPath.erase(posOfBuildFolder);
	return (binaryPath);
}

std::string WindowsPlatformFileSystem::MakeModuleDirectoryPath()
{
	HMODULE currentModuleHandle;
	GetModuleHandleExA(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR)&WindowsPlatformFileSystem::MakeModuleDirectoryPath,
		&currentModuleHandle
	);

	if (currentModuleHandle == NULL)
		return ("");

	char currentModulePath[MAX_PATH];
	GetModuleFileNameA(currentModuleHandle, currentModulePath, MAX_PATH);

	// replace the last '\' with '\0' to crop the module name
	*strrchr(currentModulePath, '\\') = '\0';

	return (currentModulePath);
}
