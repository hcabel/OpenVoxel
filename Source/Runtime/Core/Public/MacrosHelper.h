#pragma once

#include <assert.h>

// Remove unused warning
#define UNUSED(x) (void)(x)

// placeholder for removed macro
#define EMPTY_MACRO

#define CHECK(x) assert(x)

#define STATIC_CHECK_TYPE(Type, Variable) static_assert(std::is_same<Type, decltype(Variable)>::value, #Variable " must be a " #Type)
#define STATIC_CHECKF_TYPE(Type, Variable, Format, ...) static_assert(std::is_same<Type, decltype(Variable)>::value, std::format(Format, __VA_ARGS__))

# define FOR_EACH_LOOP(Variable, Array) for (auto& Variable : Array)
