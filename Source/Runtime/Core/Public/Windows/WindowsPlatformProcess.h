#pragma once

#include "Core_API.h"

#include <processthreadsapi.h>
#include <cstdint>

/**
 * class will provide information about the current process
 * 
 * /!\ WINDOWS implementation /!\
 */
class CORE_API WindowsPlatformProcess
{
public:
	/** Return the PID of the currently running process. */
	static uint32_t Pid() { GetCurrentProcessId(); }
};

typedef WindowsPlatformProcess PlatformProcess;

