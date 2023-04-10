#pragma once

#include "MacrosHelper.h"
#include "Logging/LoggingMacros.h"
#include "Profiling/ProfilingMacros.h"

DECLARE_LOG_CATEGORY(LogVulkan);

#define CHECK_VULKAN_RESULT(result, message) \
	if (result != vk::Result::eSuccess) \
	{ \
		OV_LOG(Error, LogVulkan, "Vulkan Error: {} - {}", message, vk::to_string(result)); \
	}
