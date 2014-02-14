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
    //  * bit16: the number of bytes used
    uint16_t used_bytes = 0;
    m_fsfile.readsome((char *)&used_bytes, sizeof(uint16_t));
    if(used_bytes <= 0) {
        std::cerr << "This is not a valid fs. abort." << std::endl;
        std::abort();
    }//if
    used_bytes &= 0x7FFF;

    std::vector<char> ret;
    ret.resize(used_bytes);

    m_fsfile.readsome(ret.data(), used_bytes);
    int32_t * p32bits = (int32_t *)ret.data();
    m_size = *p32bits;
    m_block_sz = *(int16_t *)(p32bits + 1);

    if(m_block_sz < 64) {
        std::cerr << "FileSystem error: block size too small, corrupted fs." << std::endl;
        std::abort();
    }//if
}//FSCore(fsfile)

std::vector<char>
FSCore::blockread(int16_t blockid, std::size_t blocknum) {
    // @TODO reading a truck of blocks together
    
    m_fsfile.seekg(m_block_sz * blockid, std::fstream::beg);

    // first 16 bits stores block info:
    //  * bit16: the number of bytes used
    uint16_t used_bytes = 0;
    m_fsfile.readsome((char *)&used_bytes, sizeof(uint16_t));

    // if this block is not used
    if(used_bytes == 0)
        return std::vector<char>();

    std::vector<char> ret;
    ret.resize(used_bytes);

    m_fsfile.readsome(ret.data(), used_bytes);

    m_fsfile.seekg(0, std::fstream::beg);

    return ret;
}//blockread(blockid)

void
FSCore::blockwrite(int16_t blockid, const std::vector<char> & data) {
    if(!check_range(blockid)) {
        std::cerr << "blockid: " << blockid << " out of range" << std::endl;
        std::abort();
    }//if

    if(data.size()-2 > (std::size_t)m_block_sz) {
        std::cerr << "FSCore blockwrite error: block too large" << std::endl;
        std::abort();
    }//if

    m_fsfile.seekp(m_block_sz * blockid, std::fstream::beg);

    // first 16 bits are:
    //  * bit16: the number of bytes used
    uint16_t sz = (uint16_t)data.size();

    m_fsfile.write((char *)&sz, sizeof(uint16_t));
    m_fsfile.write(data.data(), data.size());

    m_fsfile.flush();
    m_fsfile.seekp(0, std::fstream::beg);
}//blockwrite(blockid, data)

void
FSCore::blockformat(int16_t blockid) {
    if(!check_range(blockid)) {
        std::cerr << "blockid: " << blockid << " out of range" << std::endl;
        std::abort();
    }//if

    m_fsfile.seekp(m_block_sz * blockid, std::fstream::beg);

    char blankdata[m_block_sz];
    std::fill(blankdata, blankdata + m_block_sz, 0);
    m_fsfile.write(blankdata, m_block_sz);

    m_fsfile.flush();
    m_fsfile.seekp(0, std::fstream::beg);
}//blockformat(blockid)

bool
FSCore::blockused(int16_t blockid) {
    if(!check_range(blockid)) {
        std::cerr << "blockid: `" << blockid << "' out of range" << std::endl;
        std::abort();
    }//if
    
    m_fsfile.seekg(m_block_sz * blockid, std::fstream::beg);

    uint16_t checkread;
    m_fsfile.readsome((char *)&checkread, sizeof(uint16_t));
    
    m_fsfile.seekg(0, std::fstream::beg);

    return (checkread > 0);
}//blockused(blockid)

bool
FSCore::check_range(int16_t blockid) const {
    return (blockid < m_size/m_block_sz) && (blockid >= 0);
}//check_range(blockid)

}//namespace afs

