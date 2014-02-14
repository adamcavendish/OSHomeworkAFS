#pragma once

// STL
#include <string>
#include <algorithm>
// C-STL
#include <cstdint>
// afs
#include "ghelper.hpp"

namespace afs {

struct Env;

struct INode {
    static const int c_addrnum = 13;
    static const int c_struct_sz = (
        sizeof(int16_t) +
        sizeof(int16_t) * INode::c_addrnum);

// members
    int16_t m_blocks_num;
    int16_t m_addr[INode::c_addrnum];

// functions
    explicit INode() :
        m_blocks_num(0)
    { std::fill(m_addr, m_addr + INode::c_addrnum, 0); }

    bool
    add_block(Env & env, int16_t blockid);

    std::size_t
    size(const Env & env) const;
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

