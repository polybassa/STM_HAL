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
