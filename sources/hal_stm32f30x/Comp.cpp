// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "Comp.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Comp;
using hal::Factory;

void Comp::initialize() const
{
    COMP_DeInit(mPeripherie);
    COMP_Init(mPeripherie, &mConfiguration);
    COMP_Cmd(mPeripherie, ENABLE);
}

constexpr const std::array<const Comp, Comp::__ENUM__SIZE> Factory<Comp>::Container;
