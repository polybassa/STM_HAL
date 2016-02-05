/* Copyright (C) 2015  Nils Weiss
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef SOURCES_PMD__DEEPSLEEPINTERFACE_H_
#define SOURCES_PMD__DEEPSLEEPINTERFACE_H_

#include <vector>

namespace os
{
class DeepSleepModule {
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

class DeepSleepController {
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

#endif /* SOURCES_PMD__DEEPSLEEPINTERFACE_H_ */
