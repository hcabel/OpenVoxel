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
// Check if a variable is of a specific type
#define STATIC_CHECK_TYPE(Type, Variable) static_assert(std::is_same<Type, decltype(Variable)>::value, #Variable " must be a " #Type)
// Check if a variable is of a specific type and print a custom message
#define STATIC_CHECKF_TYPE(Type, Variable, Format, ...) static_assert(std::is_same<Type, decltype(Variable)>::value, std::format(Format, __VA_ARGS__))

// Check if a Type is based on another Type
#define STATIC_CHECK_BASE_OF(Base, Derived) static_assert(std::is_base_of<Base, Derived>::value, #Derived " must inherit from " #Base)

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
 * return: "[PlatformName]/[PlatformName]Platform[Header]"
 * e.g. "Windows/WindowsPlatformFile.h", "Mac/MacPlatformFile.h", "Linux/LinuxPlatformFile.h", ...
 */
#define PLATFORM_HEADER(Header) TO_STRING(JOIN(JOIN(PLATFORM_NAME/PLATFORM_NAME, Platform), Header))
/**
 * Link a platform specific class to the generic class,
 * return: typedef [PlatformName]Platform[ClassName] [ClassName];
 * e.g. typedef WindowsPlatformFile File;
 */
#define LINK_PLATFORM_CLASS(ClassName) typedef JOIN(JOIN(PLATFORM_NAME, Platform), ClassName) ClassName;

/* Compiler macros */

#define OV_DLL_IMPORT __declspec(dllimport)
#define OV_DLL_EXPORT __declspec(dllexport)

/* Useful constant */

#ifdef PLATFORM_WINDOWS
# define MAX_PATH_LENGTH 260 // Has describe in https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation (dont want to include windows.h here too heavy)
#else
# define MAX_PATH_LENGTH 512 // Arbitrary value, should be enough, I have no idea what is the max path length on Linux/Mac
#endif
