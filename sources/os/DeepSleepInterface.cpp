// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "DeepSleepInterface.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using os::DeepSleepModule;

std::vector<DeepSleepModule*> os::DeepSleepModule::Modules;

DeepSleepModule::DeepSleepModule(void)
{
    Modules.emplace_back(this);
}

DeepSleepModule::~DeepSleepModule(void)
{
    for (auto it = Modules.begin(); it != Modules.end(); ++it) {
        if (*it == this) {
            Modules.erase(it);
            break;
        }
    }
}
