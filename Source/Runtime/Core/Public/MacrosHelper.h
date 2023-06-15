#pragma once

#include <assert.h>

// Remove unused warning
#define UNUSED(x) (void)(x)

// placeholder for removed macro
#define EMPTY_MACRO

#define JOIN(Left, Right) JOIN_IMPL(Left, Right)
#define JOIN_IMPL(Left, Right) Left##Right

#define TO_STRING(x) TO_STRING_IMPL(x)
#define TO_STRING_IMPL(x) #x

/* Flow Control */

#define FOR_EACH_LOOP(Variable, Array) for (auto& Variable : Array)

/* Check */

#define CHECK(x) assert(x)
#define STATIC_CHECK_TYPE(Type, Variable) static_assert(std::is_same<Type, decltype(Variable)>::value, #Variable " must be a " #Type)
#define STATIC_CHECKF_TYPE(Type, Variable, Format, ...) static_assert(std::is_same<Type, decltype(Variable)>::value, std::format(Format, __VA_ARGS__))

/* Platform macros */

#if PLATFORM_WINDOWS
# define PLATFORM_NAME Windows
#elif PLATFORM_MAC
# define PLATFORM_NAME Mac
#elif PLATFORM_LINUX
# define PLATFORM_MAME Linux
#else
# error PLATFORM_NAME not defined for this platform
#endif

/**
 * Platform header will change the include path depending of the current platform.
 * 
 * "Windows = Windows/Windows[HeaderName]"
 * "Linux = Linux/Linux[HeaderName]"
 */
#define PLATFORM_HEADER(Header) TO_STRING(JOIN(PLATFORM_NAME/PLATFORM_NAME, Header))

/* Compiler macros */

#define OV_DLL_IMPORT __declspec(dllimport)
#define OV_DLL_EXPORT __declspec(dllexport)
