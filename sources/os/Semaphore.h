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

#ifndef SOURCES_PMD__SEMAPHORE_H_
#define SOURCES_PMD__SEMAPHORE_H_

#include "FreeRTOS.h"
#include "semphr.h"
#include "os_Task.h"

namespace os
{
class Semaphore
{
    SemaphoreHandle_t mSemaphoreHandle = nullptr;
    bool take(uint32_t ticksToWait) const;

public:
    Semaphore(void);
    Semaphore(const Semaphore&) = delete;
    Semaphore(Semaphore &&);
    Semaphore& operator=(const Semaphore&) = delete;
    Semaphore& operator=(Semaphore &&);
    ~Semaphore(void);

    bool take(void) const {return this->take(portMAX_DELAY); }
    template<class rep, class period>
    bool take(const std::chrono::duration<rep, period>& d) const
    {
        return take(std::chrono::duration_cast<std::chrono::milliseconds>(d).count() / portTICK_RATE_MS);
    }
    bool give(void) const;
    bool giveFromISR(void) const;
    bool takeFromISR(void) const;

    operator bool() const;
};
}

#endif /* SOURCES_PMD__SEMAPHORE_H_ */
