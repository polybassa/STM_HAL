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

template<typename T, typename U>
void hexlify(T& dest, const U& src)
{
    const char hex[] = "0123456789ABCDEF";

    dest.resize(src.size() * 2);

    auto destIt = dest.begin();

    for (const auto x : src) {
        *destIt++ = hex[(x & 0xf0) >> 4];
        *destIt++ = hex[(x & 0x0f)];
    }
}
