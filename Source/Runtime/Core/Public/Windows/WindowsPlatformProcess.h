#pragma once

#include <processthreadsapi.h>
#include <cstdint>

class WindowsPlatformProcess
{

public:
	uint32_t Pid() const { GetCurrentProcessId(); }
};

typedef WindowsPlatformProcess PlatformProcess;
