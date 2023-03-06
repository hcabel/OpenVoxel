#pragma once

#ifdef OV_BUILD_DLL
# define ENGINE_API __declspec(dllexport)
#else
# define ENGINE_API __declspec(dllimport)
#endif
