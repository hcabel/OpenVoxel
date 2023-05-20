#pragma once

#include "MacrosHelper.h"
#include "ProfilingTimer.h"
#include "ProfilingPerFrame.h"

#ifdef NO_PROFILING
// Clean up the macros

# define TO_SECONDS(x) EMPTY_MACRO
# define TO_MILLISECONDS(x) EMPTY_MACRO
# define TO_MICROSECONDS(x) EMPTY_MACRO
# define TO_NANOSECONDS(x) EMPTY_MACRO

# define TO_FLOAT_SECONDS(x) EMPTY_MACRO
# define TO_FLOAT_MILLISECONDS(x) EMPTY_MACRO
# define TO_FLOAT_MICROSECONDS(x) EMPTY_MACRO

# define TO_DOUBLE_SECONDS(x) EMPTY_MACRO
# define TO_DOUBLE_MILLISECONDS(x) EMPTY_MACRO
# define TO_DOUBLE_MICROSECONDS(x) EMPTY_MACRO

# define START_NAMED_TIMER(Name) EMPTY_MACRO
# define STOP_NAMED_TIMER(Name) EMPTY_MACRO
# define TIMER_NAMED_RESULT(Name) EMPTY_MACRO
# define TIMER_NAMED_ELAPSED(Name) EMPTY_MACRO

# define START_TIMER EMPTY_MACRO
# define STOP_TIMER EMPTY_MACRO
# define TIMER_RESULT EMPTY_MACRO
# define TIMER_ELAPSED EMPTY_MACRO

# define RESET_ALL_PERFRAME_TIMER_DATA EMPTY_MACRO
# define RESET_PERFRAME_TIMER_DATA(Category) EMPTY_MACRO

# define GET_PERFRAME_DATA(Category) EMPTY_MACRO

# define START_PERFRAME_TIMER(Category) EMPTY_MACRO
# define STOP_PERFRAME_TIMER(Category) EMPTY_MACRO

# define GET_PERFRAME_TIMER_DATA(Category) EMPTY_MACRO

# define CREATE_SCOPE_NAMED_TIMER(Name, Lamda) EMPTY_MACRO
# define CREATE_SCOPE_TIMER(Lamda) EMPTY_MACRO

# define CREATE_PERFRAME_SCOPE_NAMED_TIMER(Name, Category) EMPTY_MACRO
# define CREATE_PERFRAME_SCOPE_TIMER(Category) EMPTY_MACRO

# define CREATE_SCOPE_NAMED_TIMER_CONSOLE(Name) EMPTY_MACRO
# define CREATE_SCOPE_TIMER_CONSOLE EMPTY_MACRO

#else // NO_PROFILING

# define TO_SECONDS(x) (std::chrono::duration_cast<std::chrono::seconds>(x).count())
# define TO_MILLISECONDS(x) (std::chrono::duration_cast<std::chrono::milliseconds>(x).count())
# define TO_MICROSECONDS(x) (std::chrono::duration_cast<std::chrono::microseconds>(x).count())
# define TO_NANOSECONDS(x) (std::chrono::duration_cast<std::chrono::nanoseconds>(x).count())

# define TO_FLOAT_SECONDS(x) (static_cast<float>(TO_MILLISECONDS(x)) * 0.001f)
# define TO_FLOAT_MILLISECONDS(x) (static_cast<float>(TO_MICROSECONDS(x)) * 0.001f)
# define TO_FLOAT_MICROSECONDS(x) (static_cast<float>(TO_NANOSECONDS(x)) * 0.001f)

# define TO_DOUBLE_SECONDS(x) (static_cast<double>(TO_MILLISECONDS(x)) * 0.001)
# define TO_DOUBLE_MILLISECONDS(x) (static_cast<double>(TO_MICROSECONDS(x)) * 0.001)
# define TO_DOUBLE_MICROSECONDS(x) (static_cast<double>(TO_NANOSECONDS(x)) * 0.001)

// Basic Timer, name specific

/** Start a timer, with a specific name */
# define START_NAMED_TIMER(Name) Timer Name{}
/** Get the time elapsed since specified timer has been created */
# define TIMER_NAMED_ELAPSED(Name) Name.Elapsed()
/** Stop a specific timer (create a variable) */
# define STOP_NAMED_TIMER(Name) std::chrono::nanoseconds Name##Result = TIMER_NAMED_ELAPSED(Name)
/** Get result of a specific timer when he was stopped */
# define TIMER_NAMED_RESULT(Name) Name##Result

// Basic Timer, with predefined name

/** Start a timer */
# define START_TIMER START_NAMED_TIMER(__BasicTimer)
/** Get the time elapsed since timer has been created */
# define TIMER_ELAPSED TIMER_NAMED_ELAPSED(__BasicTimer)
/** Stop a timer (create a variable) */
# define STOP_TIMER STOP_NAMED_TIMER(__BasicTimer)
/** Get the timer result when stopped */
# define TIMER_RESULT TIMER_NAMED_RESULT(__BasicTimer)

// Per frame timer

/** Clear all per frame timer data, use at the start of each frame (you probably don't want to mess with this) */
# define CLEAR_ALL_PERFRAME_TIMER_DATA() PerFrameProfilerStorage::ClearAllData();
/** Clear a single category of per frame timer data */
# define CLEAR_PERFRAME_TIMER_DATA(Category) PerFrameProfilerStorage::ClearData(Category);

/** Get data of a per frame timer category */
# define GET_PERFRAME_DATA(Category) PerFrameProfilerStorage::GetData(Category)

/** Start a Timer that track execution on a per frame basis (How many time it's called, min/max/average time per frame) */
# define START_PERFRAME_TIMER(Category) START_NAMED_TIMER(__PerFrameNamedTimer##Category)
/**  Stop a per frame timer tracking, (and report the value to the PerFrameProfiller class) */
# define STOP_PERFRAME_TIMER(Category) PerFrameProfilerStorage::Report(#Category, TIMER_NAMED_ELAPSED(__PerFrameNamedTimer##Category))
/** Get data of a per frame timer */
# define GET_PERFRAME_TIMER_DATA(Category) GET_PERFRAME_DATA(#Category)

// Scope timer

/** Create a scope timer, this timer start at the begging of the scope and stop when the scope dies, the second argument is a function that will be call when the timer end with the elapse time has single argument (uint64_t) */
# define CREATE_SCOPE_NAMED_TIMER(Name, Lamda) ScopeTimer Name{Lamda}
/** Create a scope timer, this timer start at the begging of the scope and stop when the scope dies. Take a function that will be call when the timer end with the elapse time has single argument (uint64_t) */
# define CREATE_SCOPE_TIMER(Lamda) CREATE_SCOPE_NAMED_TIMER(__ScopeTimer, Lamda)

/** Create a scope timer, but this one automatically report his data to the PerFrameProfiler when his done (end of scope) */
# define CREATE_PERFRAME_SCOPE_NAMED_TIMER(Name, Category) ScopeTimerPerFrame Name{Category}
/** Create a scope timer, but this one automatically report his data to the PerFrameProfiler when his done (end of scope) */
# define CREATE_PERFRAME_SCOPE_TIMER(Category) CREATE_PERFRAME_SCOPE_NAMED_TIMER(__PerFrameScopedTimer, Category)

/** Create a scope timer, but this one automatically print his data to the console when his done (end of scope) */
# define CREATE_SCOPE_NAMED_TIMER_CONSOLE(Name) ConsoleScopeTimer Name{#Name}
/** Create a scope timer, but this one automatically print his data to the console when his done (end of scope) */
# define CREATE_SCOPE_TIMER_CONSOLE CREATE_SCOPE_NAMED_TIMER_CONSOLE(__ScopeTimer)

#endif // NO PROFILING
