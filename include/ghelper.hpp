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
}//print_str()

/**
 * bprint_str() print type T into binary char
 */
template <typename T>
inline std::vector<char>
bprint_str(const T & t) {
    afs_UNUSED(t);
    static_assert(sizeof(T) == -1, "binary print str of this type is not specialized");
    return std::vector<char>();
}//print_str()

}//namespace afs

