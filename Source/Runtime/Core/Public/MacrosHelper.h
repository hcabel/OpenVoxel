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

#if OV_PLATFORM_WINDOWS
# define OV_PLATFORM_NAME Windows
#elif OV_PLATFORM_MAC
# define OV_PLATFORM_NAME Mac
#elif OV_PLATFORM_LINUX
# define OV_PLATFORM_MAME Linux
#else
# error OV_PLATFORM_NAME does not detect this operating system
#endif

#define PLATFORM_HEADER(Header) TO_STRING(JOIN(OV_PLATFORM_NAME/OV_PLATFORM_NAME, Header))

/* Compiler macros */

#define OV_DLL_IMPORT __declspec(dllimport)
#define OV_DLL_EXPORT __declspec(dllexport)
