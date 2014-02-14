#pragma once
// STL
#include <vector>
// C-STL
#include <cstdint>
// afs
#include "ghelper.hpp"

namespace afs {

struct FSInfo {
    static const int c_struct_sz = (
        sizeof(int32_t) +
        sizeof(int32_t));

    int32_t m_fs_size;
    int32_t m_fs_block_sz;
};//struct FSInfo

// write helper function
template <>
inline std::vector<char>
bprint_str<FSInfo>(const FSInfo & t) {
    std::vector<char> ret;
    ret.reserve(FSInfo::c_struct_sz);
    ret.insert(ret.end(), (char *)(&t.m_fs_size), (char *)(&(t.m_fs_size) + 1));
    ret.insert(ret.end(), (char *)(&t.m_fs_block_sz), (char *)(&(t.m_fs_block_sz) + 1));
    return ret;
}//bprint_str(t)

}//namespace afs
