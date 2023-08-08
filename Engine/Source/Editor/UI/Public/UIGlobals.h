#pragma once

#include "Logging/LoggingMacros.h"

DECLARE_LOG_CATEGORY(LogUI);

#define UI_LOG(Verbosity, Format, ...) \
	OV_LOG(LogUI, Verbosity, Format, __VA_ARGS__);
