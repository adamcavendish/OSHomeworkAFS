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
    blockread(uint32_t blockid, std::size_t blocknum);

    void
    blockwrite(uint32_t blockid, const std::vector<char> & data);
    
AFS_PRIVATE:
    uint32_t m_size;
    uint32_t m_block_sz;
    std::fstream m_fsfile;
};//class FSCore

}//namespace afs

