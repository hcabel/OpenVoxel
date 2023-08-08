#pragma once

#include "MacrosHelper.h"

#ifdef OV_BUILD_CORE_DLL
# define CORE_API OV_DLL_EXPORT
#else
# define CORE_API OV_DLL_IMPORT
#endif

