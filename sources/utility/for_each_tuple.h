// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <cstddef>
#include <tuple>
#include <utility>

template<typename Tuple, typename F, std::size_t ... Indices>
void for_each_impl(Tuple&& tuple, F&& f, std::index_sequence<Indices ...> )
{
    using swallow = int[];
    (void)swallow {
        1,
        (f(std::get<Indices>(std::forward<Tuple>(tuple))), void(), int {}) ...
    };
}

template<typename Tuple, typename F>
void for_each(Tuple&& tuple, F&& f)
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
