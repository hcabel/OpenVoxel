#pragma once

#include "Core_API.h"
#include "Path.h"

#include <windows.h>

class CORE_API WindowsPlatformDLLFile
{
private:
	WindowsPlatformDLLFile(HMODULE dllHandle)
		: m_Handle(dllHandle)
	{}

public:
	__forceinline static WindowsPlatformDLLFile* Load(const Path& modulePath)
	{
		return (new WindowsPlatformDLLFile(LoadLibraryA(modulePath.GetPath().c_str())));
	}

	inline void* GetFunctionPtr(const char* functionName) { return GetProcAddress(m_Handle, functionName); }

public:
	__forceinline bool IsValid() const { return m_Handle != nullptr; }

private:
	const HMODULE m_Handle;
};

typedef WindowsPlatformDLLFile PlatformDLLFile;
