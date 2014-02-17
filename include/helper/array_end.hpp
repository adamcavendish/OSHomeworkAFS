#pragma once
// STL
#include <type_traits>

/**
 * @brief see ghelper.hpp: array_end function for reference.
 */

namespace afs {

template<typename T>
struct __array_end_helper {
    static_assert(sizeof(T) == -1, "Invalid using of `array_end` on non-array types");
};

template<typename T, std::size_t N>
struct __array_end_helper<T[N]> { enum { value = N }; };

template <std::size_t N>
struct __array_end_helper<char[N]> { enum { value = N-1 }; };

template<typename T>
struct __array_end : __array_end_helper<typename std::remove_cv<T>::type> {};

}//namespace afs

