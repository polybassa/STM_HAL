// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "os_Task.h"

namespace os
{
struct TaskEndless :
    public Task {
    using Task::Task;
};
}
