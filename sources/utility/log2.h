// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

template<size_t n>
constexpr size_t constexpr_log2(void)
{
    static_assert(n % 2 == 0 || n < 2, "Can not apply compile time log2");
    return (n < 2) ? 0 : 1 + constexpr_log2<n / 2>();
}
