// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <vector>

namespace os
{
class DeepSleepModule
{
    static std::vector<DeepSleepModule*> Modules;

protected:

    DeepSleepModule(void);

    DeepSleepModule(const DeepSleepModule&) = delete;
    DeepSleepModule(DeepSleepModule&&) = delete;
    DeepSleepModule& operator=(const DeepSleepModule&) = delete;
    DeepSleepModule& operator=(DeepSleepModule&&) = delete;

    virtual ~DeepSleepModule(void);

    virtual void enterDeepSleep(void) {while (true) {}}
    virtual void exitDeepSleep(void) {while (true) {}}

    friend class DeepSleepController;
};

class DeepSleepController
{
public:
    static void enterGlobalDeepSleep(void)
    {
        for (DeepSleepModule* module :
             DeepSleepModule::Modules)
        {
            module->enterDeepSleep();
        }
    }
    static void exitGlobalDeepSleep(void)
    {
        for (DeepSleepModule* module :
             DeepSleepModule::Modules)
        {
            module->exitDeepSleep();
        }
    }
};
}
