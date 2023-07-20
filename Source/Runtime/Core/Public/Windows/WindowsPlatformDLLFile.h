#pragma once

#include "Core_API.h"

#include <windows.h>
#include <string_view>

class CORE_API WindowsPlatformDLLFile
{
private:
	WindowsPlatformDLLFile(HMODULE dllHandle)
		: m_Handle(dllHandle)
	{}

public:
	inline static WindowsPlatformDLLFile* Load(std::string_view moduleName)
	{
		return (new WindowsPlatformDLLFile(LoadLibraryA(moduleName.data())));
	}

	inline void* GetFunctionPtr(const char* functionName) { return GetProcAddress(m_Handle, functionName); }

public:
	__forceinline bool IsValid() const { return m_Handle != nullptr; }

private:
	const HMODULE m_Handle;
};

typedef WindowsPlatformDLLFile PlatformDLLFile;
