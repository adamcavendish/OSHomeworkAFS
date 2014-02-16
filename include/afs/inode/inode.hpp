#pragma once

// STL
#include <string>
#include <algorithm>
// C-STL
#include <cstdint>
// afs
#include "ghelper.hpp"
#include "afs/env/env.hpp"
#include "afs/fscore/fscore.hpp"

namespace afs {

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
    add_block(const Env & env, int16_t blockid);

    void
    format(const Env & env);

    std::size_t
    size() const
    { return m_blocks_num; }
};//struct INode

// write helper function
template <>
inline std::vector<char>
bprint_str<INode>(const INode & t) {
    std::vector<char> ret;
    ret.reserve(INode::c_struct_sz);
    ret.insert(ret.end(), (char *)(&(t.m_blocks_num)), (char *)(&(t.m_blocks_num) + 1));
    ret.insert(ret.end(), (char *)t.m_addr, (char *)(t.m_addr + INode::c_addrnum));
    return ret;
}//bprint_str()

template <typename Iter>
inline bool
bscan_str(INode & t, Iter beg, Iter end) {
    INode temporary;

    Iter tempiter = beg;

    std::advance(tempiter, sizeof(int16_t));
    std::copy(beg, tempiter, (char *)(&(temporary.m_blocks_num)));
    beg = tempiter;

    std::advance(tempiter, sizeof(int16_t) * INode::c_addrnum);
    std::copy(beg, tempiter, (char *)(&(temporary.m_addr)));
    beg = tempiter;

    if(beg == end) {
        t.m_blocks_num = temporary.m_blocks_num;
        std::copy(temporary.m_addr, temporary.m_addr + INode::c_addrnum, t.m_addr);
        return true;
    }//if
    return false;
}//bscan_str(t, beg, end)

/**
 * @brief a replacement for std::generate, which actually apply `unary` function to every block
 *  who may change the data. `inode` is the applying inode, and `start` is the starting index.
 *
 * @return a pair of {index, blockid}
 *
 * Sorry that we cannot use std::generate to apply on INode (not implementing its iterators),
 * so I implemented a replacement for it.
 *
 * The returning `index` indicates the index where transform function stops,
 * and the returning `blockid` is the counterpart blockid.
 *
 * It is needed that the prototype of `unary` function is:
 *      bool unary(INode & inode, int16_t & block, bool is_one_level_index);
 *
 *      `inode` is the inode that is applied your unary function
 *      `block` is the current block
 *      `is_one_level_index` is whether the current block is referenced by one level index (0-9)
 *
 *  @example: see `fs_list_directory` function in `fsinterface.cpp`
 */
template <typename Unary>
inline std::pair<std::size_t, int16_t>
transform(const Env & env, INode & inode, std::size_t start, Unary unary) {
    const std::size_t addrnums = env.m_fscore->fs_data_max_sz()/sizeof(int16_t);

    std::size_t index = 0;
    // 1-level-index
    int16_t i = 0;
    for(i = 0; i < 10; ++i, ++index) {
        if(index < start)
            continue;
        if(inode.m_addr[i] == 0)
            return std::make_pair(index, inode.m_addr[i]);
        if(!unary(inode, inode.m_addr[i], true))
            return std::make_pair(index, inode.m_addr[i]);
    }//for

    // 2-level-index
    for(i = 10; i < inode.c_addrnum; ++i, ++index) {
        auto level1 = env.m_fscore->blockread(inode.m_addr[i], 1);
        int16_t * pint16_t = (int16_t *)level1.data();
        for(std::size_t i = 0; i < addrnums; ++i) {
            if(index < start)
                continue;
            if(pint16_t[i] == 0)
                return std::make_pair(index, pint16_t[i]);
            if(!unary(inode, pint16_t[i], false))
                return std::make_pair(index, pint16_t[i]);
        }//for
    }//for

    return std::make_pair(index, -1);
}//transform(inode, start, unary)

}//namespace afs

