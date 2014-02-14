#include "afs/fscore/fscore.hpp"
// STL
#include <iostream>
#include <fstream>
#include <vector>
// C-STL
#include <cstdlib>

namespace afs {

FSCore::FSCore(const std::string & fsfile) :
    m_size(0),
    m_block_sz(0),
    m_fsfile(fsfile, std::fstream::in | std::fstream::out | std::fstream::binary)
{
    if(!m_fsfile.is_open()) {
        std::cerr << "FileSystem open error, file not exists: " << fsfile << std::endl;
        std::abort();
    }//if

    // first 16 bits stores block info:
    //  * int1: used or not
    //  * int15: the number of bytes used
    int16_t used_bytes = 0;
    m_fsfile.readsome((char *)&used_bytes, sizeof(int16_t));
    if((used_bytes & 0x8000) == 0) {
        std::cerr << "This is not a valid fs. abort." << std::endl;
        std::abort();
    }//if
    used_bytes &= 0x7FFF;

    std::vector<char> ret;
    ret.resize(used_bytes);

    m_fsfile.readsome(ret.data(), used_bytes);
    int32_t * p32bits = (int32_t *)ret.data();
    m_size = *p32bits;
    m_block_sz = *(p32bits + 1);

    if(m_block_sz < 64) {
        std::cerr << "FileSystem error: block size too small, corrupted fs." << std::endl;
        std::abort();
    }//if
}//FSCore(fsfile)

std::vector<char>
FSCore::blockread(uint32_t blockid, std::size_t blocknum) {
    // @TODO reading a truck of blocks together
    
    m_fsfile.seekg(m_block_sz * blockid, std::fstream::beg);

    // first 16 bits stores block info:
    //  * int1: used or not
    //  * int15: the number of bytes used
    int16_t used_bytes = 0;
    m_fsfile.readsome((char *)&used_bytes, sizeof(int16_t));

    // if this block is not used
    if((used_bytes & 0x8000) == 0)
        return std::vector<char>();

    used_bytes &= 0x7FFF;

    std::vector<char> ret;
    ret.resize(used_bytes);

    m_fsfile.readsome(ret.data(), used_bytes);

    m_fsfile.seekg(0, std::fstream::beg);

    return ret;
}//blockread(blockid)

void
FSCore::blockwrite(uint32_t blockid, const std::vector<char> & data) {
    if(data.size()-2 > m_block_sz) {
        std::cerr << "FSCore blockwrite error: block too large" << std::endl;
        std::abort();
    }//if

    m_fsfile.seekp(m_block_sz * blockid, std::fstream::beg);

    // first 16 bits are:
    //  * int1: used or not
    //  * int15: the number of bytes used
    int16_t sz = (int16_t)data.size();
    sz |= 0x8000; // highest

    m_fsfile.write((char *)&sz, sizeof(int16_t));
    m_fsfile.write(data.data(), data.size());
}//blockwrite(blockid, data)

}//namespace afs

