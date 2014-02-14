#pragma once

// STL
#include <string>
// C-STL
#include <cstdint>
// afs
#include "ghelper.hpp"

namespace afs {

struct INode {
    static const int c_addrnum = 13;
    static const int c_struct_sz = (
        sizeof(int16_t) +
        sizeof(int16_t) * INode::c_addrnum);

    int16_t m_blocks_num;
    int16_t m_addr[INode::c_addrnum];
};//struct INode

// write helper function
template <>
inline std::vector<char>
bprint_str<INode>(const INode & t) {
    std::vector<char> ret;
    ret.reserve(sizeof(int8_t) + sizeof(int16_t) * INode::c_addrnum);
    ret.insert(ret.end(), (char *)(&(t.m_blocks_num)), (char *)(&(t.m_blocks_num) + 1));
    ret.insert(ret.end(), (char *)t.m_addr, (char *)(t.m_addr + INode::c_addrnum));
    return ret;
}//bprint_str()

}//namespace afs

