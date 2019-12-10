/// @file returnTypeDeduction.h
/// @brief Helper functions to deduct the return type at compile time.
/// @author Henning Mende (henning@my-urmo.com)
/// @date   Sep 12, 2019
/// @copyright UrmO GmbH
///
/// This program is free software: you can redistribute it and/or modify it under the terms
/// of the GNU General Public License as published by the Free Software Foundation, either
/// version 3 of the License, or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
/// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/// See the GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License along with this program.
/// If not, see <https://www.gnu.org/licenses/>.
///
#ifndef SOURCES_UTILITY_RETURNTYPEDEDUCTION_H_
#define SOURCES_UTILITY_RETURNTYPEDEDUCTION_H_

namespace utility
{
/// @brief Helper to determine the return type of a function.
///
/// This can be used to determine the return type of overloaded or templated functions at
/// compile time, even if the functions are not `constexpr`.
///
/// Usage:
/// @code
/// int foo(float a);
/// float foo(int a);
/// decltype(getReturnType<float>(foo)) b = 3;
/// @endcode
///
/// In this example b is int.
///
/// @tparam R Return type (evaluated automatically).
/// @tparam A Argument types for the target function (for overloaded functions).
/// @param * Function with @a A parameters.
/// @return Type of the return value of @a *.
template<typename R, typename ... A>
R getReturnType(R (*)(A ...));

/// @brief Helper to determine the return type of a method.
///
/// This can be used to determine the return type of overloaded or templated methods at compile
/// time, even if the methods are not `constexpr`.
///
/// Usage:
/// @code
/// struct Foo {
///     int foo(float a);
///     float foo(int a);
///    } fooInstance;
///    decltype(utility::getReturnType<float>(& Foo::foo)) b = 3;
///    decltype(utility::getReturnType<float>(& decltype(fooInstance)::foo)) c = 3;
/// @endcode
///
/// In this example b and c are int.
///
/// @tparam M Matching version of the overloaded method (evaluated by template parameter @a A).
/// @tparam C Class name (evaluated by argument).
/// @tparam R Return type (evaluated automatically).
/// @tparam A Argument types for the target method (for overloaded methods).
/// @param * Method with @a A parameters.
/// @return Type of the return value of @a *.
template<typename M, typename C, typename R, typename ... A>
R getReturnType(R (C::*)(M, A ...));

/// @brief Helper to determine the return type of a const method.
///
/// This can be used to determine the return type of overloaded or templated methods at compile
/// time, even if the methods are not `constexpr`.
///
/// Usage:
/// @code
/// struct Foo {
///     int foo(float a) const;
///     float foo(int a) const;
///    } fooInstance;
///    decltype(utility::getReturnType<float>(& Foo::foo)) b = 3;
///    decltype(utility::getReturnType<float>(& decltype(fooInstance)::foo)) c = 3;
/// @endcode
///
/// In this example b and c are int.
///
/// @tparam V Matching version of the overloaded method (evaluated by template parameter @a A).
/// @tparam C Class name (evaluated by argument).
/// @tparam R Return type (evaluated automatically).
/// @tparam A Argument types for the target method (for overloaded methods).
/// @param * Method with A parameters.
/// @return Type of the return value of @a *.
template<typename V, typename C, typename R, typename ... A>
R getReturnType(R (C::*)(V, A ...) const);
} // namespace utility

#endif // SOURCES_UTILITY_RETURNTYPEDEDUCTION_H_
