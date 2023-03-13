#pragma once

#if OV_BUILD_DLL
# define CORE_API __declspec(dllexport)
#else
# define CORE_API __declspec(dllimport)
#endif
