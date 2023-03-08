#pragma once

#include "ProfilingTimer.h"
#include "ProfilingPerFrame.h"

#define OV_PROFILLING

#ifdef OV_PROFILLING

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

/** Reset all per frame timer data, use at the start of each frame (you probably don't want to mess with this) */
# define RESET_ALL_PERFRAME_TIMER_DATA PerFrameProfiler::ResetAllData();
/** Reset a single category of per frame timer data */
# define RESET_PERFRAME_TIMER_DATA(Category) PerFrameProfiler::ResetData(Category);

/** Get data of a per frame timer category */
# define GET_PERFRAME_DATA(Category) PerFrameProfiler::GetData(Category)

/** Start a Timer that track execution on a per frame basis (How many time it's called, min/max/average time per frame) */
# define START_PERFRAME_TIMER(Category) START_NAMED_TIMER(__PerFrameNamedTimer##Category)
/**  Stop a per frame timer tracking, (and report the value to the PerFrameProfiller class) */
# define STOP_PERFRAME_TIMER(Category) PerFrameProfiler::Report(#Category, TIMER_NAMED_ELAPSED(__PerFrameNamedTimer##Category))
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

#else
// Clean up the macros

# define TO_SECONDS(x)
# define TO_MILLISECONDS(x)
# define TO_MICROSECONDS(x)
# define TO_NANOSECONDS(x)

# define TO_FLOAT_SECONDS(x)
# define TO_FLOAT_MILLISECONDS(x)
# define TO_FLOAT_MICROSECONDS(x)

# define TO_DOUBLE_SECONDS(x)
# define TO_DOUBLE_MILLISECONDS(x)
# define TO_DOUBLE_MICROSECONDS(x)

# define START_NAMED_TIMER(Name)
# define STOP_NAMED_TIMER(Name)
# define TIMER_NAMED_RESULT(Name)
# define TIMER_NAMED_ELAPSED(Name)

# define START_TIMER
# define STOP_TIMER
# define TIMER_RESULT
# define TIMER_ELAPSED

# define RESET_ALL_PERFRAME_TIMER_DATA
# define RESET_PERFRAME_TIMER_DATA(Category)

# define GET_PERFRAME_DATA(Category)

# define START_PERFRAME_TIMER(Category)
# define STOP_PERFRAME_TIMER(Category)

# define GET_PERFRAME_TIMER_DATA(Category)

# define CREATE_SCOPE_NAMED_TIMER(Name, Lamda)
# define CREATE_SCOPE_TIMER(Lamda)

# define CREATE_PERFRAME_SCOPE_NAMED_TIMER(Name, Category)
# define CREATE_PERFRAME_SCOPE_TIMER(Category)

# define CREATE_SCOPE_NAMED_TIMER_CONSOLE(Name)
# define CREATE_SCOPE_TIMER_CONSOLE
#endif
