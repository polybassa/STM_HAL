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

#ifndef SOURCES_PMD__COUNTINGSEMAPHORE_H_
#define SOURCES_PMD__COUNTINGSEMAPHORE_H_

#include "FreeRTOS.h"
#include "semphr.h"
#include "pmd_Task.h"

namespace os
{
class CountingSemaphore {
    SemaphoreHandle_t mSemaphoreHandle = nullptr;

public:
    CountingSemaphore(uint32_t maximalCount, uint32_t initalCount);
    CountingSemaphore(const CountingSemaphore&) = delete;
    CountingSemaphore(CountingSemaphore&&);
    CountingSemaphore& operator=(const CountingSemaphore&) = delete;
    CountingSemaphore& operator=(CountingSemaphore&&);
    ~CountingSemaphore(void);

    bool take(uint32_t ticksToWait = portMAX_DELAY) const;
    bool give(void) const;
    bool giveFromISR(void) const;
    bool takeFromISR(void) const;

    operator bool() const;
};
}

#endif /* SOURCES_PMD__COUNTINGSEMAPHORE_H_ */
