// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <array>
#include <algorithm>

template<typename T, size_t N>
void applyMedianFilter(std::array<T, N>& array)
{
    static_assert(N >= 5, "Array size is not big enough to apply median filter !");

    using it = typename std::array<T, N>::iterator;

    it begin = array.begin();
    it end = array.end();

    it first = begin;
    it middle = ++begin;
    it last = ++begin;

    auto storage1 = *middle;
    auto storage2 = *last;

    it output = middle;

    auto getMedian = [](const it& a, const it& b, const it& c){
                         return std::max(std::min(*a, *b), std::min(std::max(*a, *b), *c));
                     };

    //first step for preparation
    storage1 = getMedian(first++, middle++, last++);
    //second step for preparation
    storage2 = storage1;
    storage1 = getMedian(first++, middle++, last++);

    //loop
    while (last != end) {
        *output++ = storage2;
        storage2 = storage1;
        storage1 = getMedian(first++, middle++, last++);
    }
    // flush pipeline
    *output++ = storage2;
    *output = storage1;
}
