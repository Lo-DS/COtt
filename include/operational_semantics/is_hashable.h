//
// Created by giacomo on 24/06/24.
//

#ifndef OPERATIONAL_SEMANTICS_IS_HASHABLE_H
#define OPERATIONAL_SEMANTICS_IS_HASHABLE_H

#include <functional>
#include <ios>
#include <iostream>
#include <type_traits>

///https://stackoverflow.com/a/51915825
template <typename T, typename = std::void_t<>>
struct is_std_hashable : std::false_type { };

///https://stackoverflow.com/a/51915825
template <typename T>
struct is_std_hashable<T, std::void_t<decltype(std::declval<std::hash<T>>()(std::declval<T>()))>> : std::true_type { };

///https://stackoverflow.com/a/51915825
template <typename T>
constexpr bool is_std_hashable_v = is_std_hashable<T>::value;

#endif //OPERATIONAL_SEMANTICS_IS_HASHABLE_H
