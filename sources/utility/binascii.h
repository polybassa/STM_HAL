// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <type_traits>
#include <string>
#include <string_view>

//specialize a type for all dynamic size STL containers.
namespace is_dynamic_stl_container_impl
{
template<typename T>
struct is_stl_container :
    std::false_type {};
template<typename ... Args>
struct is_stl_container<std::vector<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::deque<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::list<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::forward_list<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::set<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::multiset<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::map<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::multimap<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::unordered_set<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::unordered_multiset<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::unordered_map<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::unordered_multimap<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::stack<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::queue<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::priority_queue<Args ...> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::basic_string<Args ...> > :
    std::true_type {};
}

template<typename T>
struct is_dynamic_stl_container {
    static constexpr bool const value = is_dynamic_stl_container_impl::is_stl_container<std::decay_t<T> >::value;
};

//specialize a type for all fixed size STL containers.
namespace is_fixed_stl_container_impl
{
template<typename T>
struct is_stl_container :
    std::false_type {};
template<typename T, std::size_t N>
struct is_stl_container<std::array<T, N> > :
    std::true_type {};
template<typename ... Args>
struct is_stl_container<std::basic_string_view<Args ...> > :
    std::true_type {};
}

template<typename T>
struct is_fixed_stl_container {
    static constexpr bool const value = is_fixed_stl_container_impl::is_stl_container<std::decay_t<T> >::value;
};

template<typename V, typename W>
typename std::enable_if_t<is_fixed_stl_container<V>::value> hexlify(V& dest, const W& src)
{
    const char hex[] = "0123456789ABCDEF";

    static_assert(dest.size() == src.size() * 2);

    auto destIt = dest.begin();

    for (const auto x : src) {
        *destIt++ = hex[(x & 0xf0) >> 4];
        *destIt++ = hex[(x & 0x0f)];
    }
}

template<typename V, typename W>
typename std::enable_if_t<is_dynamic_stl_container<V>::value> hexlify(V& dest, const W& src)
{
    const char hex[] = "0123456789ABCDEF";

    dest.resize(src.size() * 2);

    auto destIt = dest.begin();

    for (const auto x : src) {
        *destIt++ = hex[(x & 0xf0) >> 4];
        *destIt++ = hex[(x & 0x0f)];
    }
}
