#include "afs/fsinterface/fsinterface.hpp"
// STL
#include <fstream>
#include <iostream>
#include <memory>
// C-STL
#include <cstdio>
// afs
#include "afs/attribute/attribute.hpp"
#include "afs/fsinfo/fsinfo.hpp"

namespace afs {

/**
 * create a file/directory depending on the flag(dir or not)
 * return true on success (create on not exist)
 * return false on failure (there's a file/dir with the same name)
 */
bool
fs_create(Env & env, const std::string & filename, int8_t owner_uid, const AttrFlag & flag) {
    // @TODO
    if(filename.find('/', 0) != std::string::npos) {
        std::cerr << "fs_create should not create a file/dir by a absolute path" << std::endl;
        std::abort();
    }//if

    Attribute attr;
    attr.m_flag = flag;
    attr.m_owner_uid = owner_uid;
    attr.m_node_name = filename;

    return false;
}//fs_create(env, filename, owner_uid, flag)

bool
fs_delete(Env & env, const std::string & filename);

std::pair<bool, std::vector<char>>
fs_read(Env & env, int32_t fd, uint64_t size);

bool
fs_write(Env & env, int32_t fd, const std::vector<char> & data);

int32_t
fs_open(Env & env, const std::string & filename, const AttrFlag & flag);

bool
fs_close(int32_t fd);

bool
fs_init(const std::string & fs_filename, int32_t fs_size, int32_t fs_block_size) {
    std::ofstream ofs(fs_filename, std::ofstream::binary);
    if(!ofs.is_open()) {
        std::cerr << "fs_init error: file not found: " << fs_filename << std::endl;
        return false;
    }//if

    {
        ofs.seekp(0, std::ios_base::beg);
        FSInfo info;
        info.m_fs_size = fs_size;
        info.m_fs_block_sz = fs_block_size;

        // write FSInfo at the beginning as a block
        int16_t info_sz = 0x8000 | FSInfo::c_struct_sz;
        ofs.write((char *)&info_sz, sizeof(int16_t));

        auto info_binary = bprint_str(info);
        ofs.write(info_binary.data(), info_binary.size());
    }//plain block

    {
        ofs.seekp(1 * fs_block_size, std::ios_base::beg);
        // write '/' root dir attributes
        Attribute rootattr;
        rootattr.m_flag = AttrFlag(AttrFlag::afs_dir | AttrFlag::afs_read | AttrFlag::afs_write);
        rootattr.m_owner_uid = int8_t(0); // root's uid is 0
        rootattr.m_inode = std::make_shared<INode>();
        rootattr.m_inode->m_blocks_num = 0;
        rootattr.m_node_name = "/";

        int16_t rootattr_sz = 0x8000 | Attribute::c_struct_sz;
        ofs.write((char *)&rootattr_sz, sizeof(int16_t));

        auto rootattr_binary = bprint_str(rootattr);
        ofs.write(rootattr_binary.data(), rootattr_binary.size());
    }//plain block

    {
        // reserve the disk space for the filesystem
        ofs.seekp(fs_size, std::ios_base::beg);
        int32_t dummy = 0;
        ofs.write((char *)&dummy, sizeof(int32_t));
    }//plain block

    ofs.flush();
    ofs.close();

    return true;
}//fs_init(fs_filename, fs_size, fs_block_sz)

}//namespace afs

