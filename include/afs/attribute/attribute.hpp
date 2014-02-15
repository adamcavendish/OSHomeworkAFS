#pragma once

// STL
#include <string>
#include <memory>
#include <algorithm>
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
            sizeof(char) * 15);

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
}//bprint_str(t)

template <typename Iter>
inline bool
bscan_str(Attribute & t, Iter beg, Iter end) {
    // temporaries for rolling back
    AttrFlag flag;
    int8_t owner_uid;
    auto inode = std::make_shared<INode>();
    std::string node_name;

    Iter tempiter = beg;

    std::advance(tempiter, sizeof(AttrFlag));
    std::copy(beg, tempiter, (char *)&flag);
    beg = tempiter;

    std::advance(tempiter, sizeof(int8_t));
    std::copy(beg, tempiter, (char *)&owner_uid);
    beg = tempiter;

    std::advance(tempiter, sizeof(INode));
    bscan_str(*inode, beg, tempiter);
    beg = tempiter;

    char name[15];
    std::advance(tempiter, sizeof(char) * 15);
    std::copy(beg, tempiter, name);
    node_name = std::string(name);
    beg = tempiter;

    if(beg == end) {
        t.m_flag = flag;
        t.m_owner_uid = owner_uid;
        t.m_inode = inode;
        t.m_node_name = node_name;
        return true;
    }//if

    return false;
}//bscan_str(t, data)

}//namespace afs

