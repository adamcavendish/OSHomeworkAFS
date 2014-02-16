#pragma once
// STL
#include <vector>
#include <string>

#define afs_UNUSED(ident) (void)(ident)

namespace afs {

/**
 * print_str() print type T into string
 */
template <typename T>
inline std::string
print_str(const T & t) {
    afs_UNUSED(t);
    static_assert(sizeof(T) == -1, "print str of this type is not specialized");
    return std::string();
}//print_str(t)

/**
 * bprint_str() print type T into binary char
 */
template <typename T>
inline std::vector<char>
bprint_str(const T & t) {
    afs_UNUSED(t);
    static_assert(sizeof(T) == -1, "binary print str of this type is not specialized");
    return std::vector<char>();
}//bprint_str(t)

/**
 * @brief bscan_str() reverse action of bprint_str,
 * which scans a from iterator `beg` to `end` into a T type
 *
 * `commit or roll back semantics` must be applied to `t`
 */
template <typename T, typename Iter>
inline bool
bscan_str(T & t, Iter beg, Iter end) {
    afs_UNUSED(t);
    afs_UNUSED(beg);
    afs_UNUSED(end);
    static_assert(sizeof(T) == -1, "binary scan str of this type is not specialized");
    return false;
}//bscan_str(t, data)

/**
 * @brief get an array's end as a pointer. Just a helper function.
 */
template <typename T, std::size_t N>
inline T *
array_end(T (&arr)[N])
{ return arr + N; }

}//namespace afs

