#pragma once

#include "MacrosHelper.h"

#ifdef OV_BUILD_EDITOR_DLL
# define EDITOR_API OV_DLL_EXPORT
#else
# define EDITOR_API OV_DLL_IMPORT
#endif
