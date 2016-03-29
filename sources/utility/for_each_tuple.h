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

#ifndef SOURCES_UTILITY_FOR_EACH_TUPLE_H_
#define SOURCES_UTILITY_FOR_EACH_TUPLE_H_

#include <tuple>
#include <utility>

template<std::size_t I = 0, typename FuncT, typename ... Tp>
inline typename std::enable_if<I == sizeof ... (Tp), void>::type
for_each(std::tuple<Tp ...>&, FuncT) // Unused arguments are given no names.
{ }

template<std::size_t I = 0, typename FuncT, typename ... Tp>
inline typename std::enable_if < I<sizeof ... (Tp), void>::type
for_each(std::tuple<Tp ...>& t, FuncT f)
{
    f(std::get<I>(t));
    for_each<I + 1, FuncT, Tp ...>(t, f);
}

template<size_t index, typename ... args>
struct pack_size_index;

template<size_t index, typename type_t, typename ... args>
struct pack_size_index<index, type_t, args ...> {
    static constexpr size_t value = (index > 0) ?
                                    (sizeof(type_t) + pack_size_index<index - 1, args ...>::value) : 0;
};

template<size_t index>
struct pack_size_index<index> {
    static constexpr size_t value = 0;
};

template<typename ... args>
struct pack_size {
    static constexpr size_t value = pack_size_index<sizeof ... (args), args ...>::value;
};

#endif /* SOURCES_UTILITY_FOR_EACH_TUPLE_H_ */
