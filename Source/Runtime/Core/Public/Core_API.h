﻿#pragma once

#include "MacrosHelper.h"

#if OV_BUILD_DLL
# define CORE_API OV_DLL_EXPORT
#else
# define CORE_API OV_DLL_IMPORT
#endif

