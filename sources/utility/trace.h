// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss, Patrick Bruenn
 */

#pragma once

#define ZONE_ERROR 0x00000001
#define ZONE_WARNING 0x00000002
#define ZONE_INFO 0x00000004
#define ZONE_VERBOSE 0x00000008

#if defined(UNITTEST)
#define Trace(ZONE, ...) do { \
        if (g_DebugZones & (ZONE)) { \
            printf("%s:%u: ", __FILE__, __LINE__); \
            printf(__VA_ARGS__); \
            fflush(stdout); \
        } \
} while (0)
#define TraceInit()
#else

#if defined(DEBUG)
#if defined(USART_DEBUG)
#include "DebugInterface.h"

static dev::DebugInterface terminal;
#else
#include "RealTimeDebugInterface.h"

static dev::RealTimeDebugInterface& terminal = dev::RealTimeDebugInterface::instance();
#endif
#define Trace(ZONE, ...) do { \
        if (g_DebugZones & (ZONE)) { \
            terminal.printf("%s:%u: ", __FILE__, __LINE__); \
            terminal.printf(__VA_ARGS__); \
        } \
} while (0)

#define TraceInit() do { \
        terminal.printStartupMessage(); \
} while (0)

#define TraceLight(...) do { \
        terminal.printf(__VA_ARGS__); \
} while (0)

#else
#define Trace(ZONE, ...)
#define TraceInit()
#endif
#endif
