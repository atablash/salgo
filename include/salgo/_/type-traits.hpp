#pragma once

#include <type_traits>

namespace salgo {




// https://akrzemi1.wordpress.com/2016/07/07/the-cost-of-stdinitializer_list/

template <bool...> struct bool_sequence {};

template <bool... Bs>
using bool_and = std::is_same<bool_sequence<Bs...>, bool_sequence<(Bs || true)...>>;

template< class T, class... Args>
using is_constructible_from_all = bool_and<std::is_constructible_v<T,Args>...>;

// # define REQUIRES(...)    class = std::enable_if_t<(__VA_ARGS__)>





}
