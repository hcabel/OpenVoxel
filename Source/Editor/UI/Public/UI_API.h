#pragma once

#include "MacrosHelper.h"

#ifdef OV_BUILD_UI_DLL
# define UI_API OV_DLL_EXPORT
#else
# define UI_API OV_DLL_IMPORT
#endif
