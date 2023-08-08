#pragma once

#include "MacrosHelper.h"

#ifdef OV_BUILD_RENDERER_DLL
# define RENDERER_API OV_DLL_EXPORT
#else
# define RENDERER_API OV_DLL_IMPORT
#endif
