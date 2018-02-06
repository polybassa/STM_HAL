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

#include <cstddef>
#include <tuple>
#include <utility>

template<typename Tuple, typename F, std::size_t ... Indices>
void for_each_impl(Tuple && tuple, F && f, std::index_sequence<Indices ...> )
{
    using swallow = int[];
    (void)swallow {
        1,
        (f(std::get<Indices>(std::forward<Tuple>(tuple))), void(), int {}) ...
    };
}

template<typename Tuple, typename F>
void for_each(Tuple && tuple, F && f)
{
    constexpr std::size_t N = std::tuple_size<std::remove_reference_t<Tuple> >::value;
    for_each_impl(std::forward<Tuple>(tuple), std::forward<F>(f),
                  std::make_index_sequence<N> {});
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
