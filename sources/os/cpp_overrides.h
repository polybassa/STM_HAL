// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#if  defined(USE_FREERTOS) && defined(__cplusplus)

#include "FreeRTOS.h"
#ifdef DEBUG
#include "SEGGER_RTT.h"
#endif

//Override C++ new/delete operators to reduce memory footprint
void* operator new(size_t size)
{
    return pvPortMalloc(size);
}

void* operator new[](size_t size)
{
    return pvPortMalloc(size);
}

void operator delete(void* p)
{
    vPortFree(p);
}

void operator delete[](void* p)
{
    vPortFree(p);
}

extern "C" void abort(void)
{
#ifdef DEBUG
    SEGGER_RTT_printf(0, "BAD SHIT Happened\r\n");
#endif
    configASSERT(0);
}

#endif
