#pragma once

#include "MacrosHelper.h"

#if OV_DLL_BUILD
# define RENDERER_API OV_DLL_EXPORT
#else
# define RENDERER_API OV_DLL_IMPORT
#endif
