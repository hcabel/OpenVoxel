#pragma once

#include "Logging/LoggingMacros.h"

DECLARE_LOG_CATEGORY(LogVulkan);
DECLARE_LOG_CATEGORY(LogVulkanInternal); // For internal Vulkan logging (such as validation layers)

#define VULKAN_LOG(Verbosity, Format, ...) OV_LOG(LogVulkan, Verbosity, Format, ##__VA_ARGS__)
#define VULKAN_LOG_ARRAY(Array, Verbosity, Format, ...) OV_LOG_ARRAY(LogVulkan, Verbosity, Array, Format, ##__VA_ARGS__)
#define VULKAN_LOG_RAW(Verbosity, Format, ...) OV_LOG_RAW(LogVulkan, Verbosity, Format, ##__VA_ARGS__)
#define VULKAN_INT_LOG(Verbosity, Format, ...) OV_LOG(LogVulkanInternal, Verbosity, Format, ##__VA_ARGS__)
