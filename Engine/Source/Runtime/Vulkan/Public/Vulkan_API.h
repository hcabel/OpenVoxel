#pragma once

#include "MacrosHelper.h"

#ifdef OV_BUILD_VULKAN_DLL
# define VULKAN_API OV_DLL_EXPORT
#else
# define VULKAN_API OV_DLL_IMPORT
#endif
