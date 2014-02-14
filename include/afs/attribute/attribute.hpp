#pragma once

// STL
#include <string>
#include <memory>
#include <iostream>
#include <vector>
// C-STL
#include <cstdlib>
// afs
#include "ghelper.hpp"
#include "afs/inode/inode.hpp"
#include "afs/attribute/flag.hpp"

namespace afs {

struct Attribute {
    static const int c_struct_sz = (
            sizeof(AttrFlag) +
            sizeof(int8_t) +
            INode::c_struct_sz +
            sizeof(int8_t) * 15);

    AttrFlag m_flag;
    int8_t m_owner_uid;
    std::shared_ptr<INode> m_inode;
    std::string m_node_name;
};//struct Attribute

// write helper function
template <>
inline std::vector<char>
bprint_str<Attribute>(const Attribute & t) {
    if(t.m_node_name.size() > 15) {
        std::cerr << "node name too long: " << t.m_node_name << std::endl;
        std::abort();
    }//if

    std::vector<char> ret;
    ret.reserve(Attribute::c_struct_sz);

    ret.insert(ret.end(), (char *)(&t.m_flag), (char *)(&(t.m_flag) + 1));
    ret.insert(ret.end(), (char *)(&t.m_owner_uid), (char *)(&(t.m_owner_uid) + 1));
    auto inode_binary = bprint_str(*(t.m_inode));
    std::move(inode_binary.begin(), inode_binary.end(), std::back_inserter(ret));

    ret.insert(ret.end(), t.m_node_name.begin(), t.m_node_name.end());
    ret.resize(Attribute::c_struct_sz);
    return ret;
}//bprint_str()

}//namespace afs

