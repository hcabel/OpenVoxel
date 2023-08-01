#include "Windows/WindowsPlatformFileSystem.h"
#include "MacrosHelper.h"
#include "Path.h"

#include <Windows.h>

Path WindowsPlatformFileSystem::MakeEngineRootDirectoryPath()
{
	// Get binary path
	char buffer[MAX_PATH_LENGTH];
	if (GetModuleFileNameA(NULL, buffer, MAX_PATH_LENGTH) == 0)
		return ("");
	Path binaryPath = buffer;

	// Remove everything from "build" to the end
	Path::size_type segmentIndex = binaryPath.RightFindSegment("build");
	if (segmentIndex != -1)
		binaryPath.TrimSegments(segmentIndex, binaryPath.GetSegmentCount() - 1);
	return (binaryPath);
}

Path WindowsPlatformFileSystem::MakeModuleDirectoryPath()
{
	HMODULE currentModuleHandle;
	GetModuleHandleExA(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR)&WindowsPlatformFileSystem::MakeModuleDirectoryPath,
		&currentModuleHandle
	);

	if (currentModuleHandle == NULL)
		return ("");

	char currentModulePath[MAX_PATH_LENGTH];
	GetModuleFileNameA(currentModuleHandle, currentModulePath, MAX_PATH_LENGTH);

	Path moduleDirectoryPath = currentModulePath;
	moduleDirectoryPath.TrimTarget();

	return (moduleDirectoryPath);
}
