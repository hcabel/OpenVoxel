#pragma once

#include "MacrosHelper.h"

#ifdef OV_BUILD_ENGINE_DLL
# define ENGINE_API OV_DLL_EXPORT
#else
# define ENGINE_API OV_DLL_IMPORT
#endif
