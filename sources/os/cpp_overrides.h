// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#if  defined(USE_FREERTOS) && defined(__cplusplus)

#include "FreeRTOS.h"
#include "trace.h"

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
    terminal.printf("BAD SHIT\r\n");
#endif
    configASSERT(0);
}

#endif
