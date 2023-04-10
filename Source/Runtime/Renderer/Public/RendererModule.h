#pragma once

#include "MacrosHelper.h"

#if OV_BUILD_DLL
# define RENDERER_API OV_DLL_EXPORT
#else
# define RENDERER_API OV_DLL_IMPORT
#endif
