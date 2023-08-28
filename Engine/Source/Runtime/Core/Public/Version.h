#pragma once

#include "MacrosHelper.h"

/**
 * Version are stored into a uint32_t with 3 component (major, minor, patch).
 * each component is 10 bits which means the maximum value of each component is 1023 (0x3FF).
 *
 * Major: Signifies significant changes or updates.
 * Minor: Represents improvement and additions to the major version.
 * Patch: Indicates patches or fixes of the minor version
 */

// Get the major version of the current version
#define OV_VERSION_GET_MAJOR(version) (((version) >> 22) & 0x3FFu)
// Get the minor version of the current version
#define OV_VERSION_GET_MINOR(version) (((version) >> 12) & 0x3FFu)
// Get the patch version of the current version
#define OV_VERSION_GET_PATCH(version) (((version) >> 2) & 0x3FFu)

#define CAST_U32(value) ((uint32_t)(value))

// Take the major, minor and patch version and make convert it to a uint32_t version
#define OV_MAKE_VERSION(major, minor, patch) (((CAST_U32(major) & 0x3FFu) << 20) | ((CAST_U32(minor) & 0x3FFu) << 10) | ((CAST_U32(patch) & 0x3FFu)) << 2)

/* OPEN VOXEL CURRENT VERSION */

#define OV_CURRENT_VERSION_MAJOR 0u // The current major version of Open Voxel
#define OV_CURRENT_VERSION_MINOR 0u // The current minor version of Open Voxel
#define OV_CURRENT_VERSION_PATCH 0u // The current patch version of Open Voxel

// The current version of Open Voxel
#define OV_CURRENT_VERSION OV_MAKE_VERSION(OV_CURRENT_VERSION_MAJOR, OV_CURRENT_VERSION_MINOR, OV_CURRENT_VERSION_PATCH)

/* ************************* */

// Tell whether or the the version x has been reached/overpassed or not
#define OV_VERSION(Major, Minor) (OV_CURRENT_VERSION >= OV_MAKE_VERSION(Major, Minor, 0))
// Tell whether or the the version x has been reached/overpassed or not (precise version)
#define OV_VERSION_PRECISE(Major, Minor, Patch) (OV_CURRENT_VERSION >= OV_MAKE_VERSION(Major, Minor, Patch))

// Subtract 2 version and return the offset
#define OV_VERSION_OFFSET(VersionA, VersionB) (VersionA - VersionB)
// Tell whether or not the two version are at least the given offset apart
#define OV_VERSION_LIMMIT_OFFSET(VersionA, VersionB, Offset) (OV_VERSION_OFFSET(VersionA, VersionB) >= Offset)

/* DEPRECATION MACROS */

// The deprecation limit is the version after which the element need to be removed
#define OV_DEPRECATION_LIMIT OV_MAKE_VERSION(1, 0, 0)
// The error message to display when the deprecation limit has been reach
#define OV_DEPRECATION_LIMIT_ERROR_MSG "The deprecation limit has been reach for this element please remove it or update the version (not recommended)"
// Tell whether or not the given version has reached the deprecation limit
#define HAS_REACHED_DEPRECATION_LIMIT(VersionDeprecatedSince) OV_VERSION_LIMMIT_OFFSET(OV_CURRENT_VERSION, VersionDeprecatedSince, OV_DEPRECATION_LIMIT)

// Tell that the function bellow is deprecated since a given version
// A deprecated function will trigger a warning when used
// additonally this macro will trigger an error when this function has been written deprecated for too long
// (the deprecation limit is defined in the OV_DEPRECATION_LIMIT macro)
//
// Example of use:
//    DEPRECATED(1, 2, "Use 'NewFunction' instead")
//    void OldFunction() { ... }
//
// Major, Is the major version since the function is deprecated
// Minor, Is the minor version since the function is deprecated
// Message, (optional) A message to tell what to use instead
#define OV_DEPRECATED(Major, Minor, Message) \
	static_assert(!HAS_REACHED_DEPRECATION_LIMIT(OV_MAKE_VERSION(Major, Minor, 0)), OV_DEPRECATION_LIMIT_ERROR_MSG); \
	[[deprecated("Deprecated since "#Major"."#Minor": "##Message" Please update your code, otherwise one day your code wont be compiling anymore")]]

/* ****************** */
