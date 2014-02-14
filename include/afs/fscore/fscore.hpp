#pragma once
// STL
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
// C-STL
#include <cstdio>
// afs
#include "gconfig.hpp"

namespace afs {

class FSCore {
AFS_PUBLIC:
    FSCore(const std::string & fsfile);
    ~FSCore() {}

    std::vector<char>
    blockread(int16_t blockid, std::size_t blocknum);

    void
    blockwrite(int16_t blockid, const std::vector<char> & data);

    void
    blockformat(int16_t blockid);

    bool
    blockused(int16_t blockid);

    int32_t
    fs_size() const
    { return m_size; }

    int16_t
    fs_block_sz() const
    { return m_block_sz; }

    int16_t
    fs_data_max_sz() const
    { return m_block_sz - cm_blockinfo_sz; }

AFS_PRIVATE:
    bool
    check_range(int16_t blockid) const;
    
AFS_PRIVATE:
    const int16_t cm_blockinfo_sz = 2;

    int32_t m_size;
    int16_t m_block_sz;
    std::fstream m_fsfile;
};//class FSCore

}//namespace afs

