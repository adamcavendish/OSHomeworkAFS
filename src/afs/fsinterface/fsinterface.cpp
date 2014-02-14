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
#include "afs/inode/inode.hpp"
#include "afs/fsinterface/fsinterface_helper.hpp"

namespace afs {

/**
 * create a file/directory depending on the flag(dir or not)
 * return true on success (create on not exist)
 * return false on failure (there's a file/dir with the same name)
 */
bool
fs_create(Env & env, const std::string & nodename, int8_t owner_uid, const AttrFlag & flag) {
    if(nodename.find('/', 0) != std::string::npos) {
        std::cerr << "fs_create should not create a file/dir by a absolute path" << std::endl;
        std::abort();
    }//if

    Attribute attr;
    attr.m_flag = flag;
    attr.m_owner_uid = owner_uid;
    attr.m_node_name = nodename;
    attr.m_inode = std::make_shared<INode>();

    auto block = alloc_one_block(env);
    if(block == -1) {
        std::cerr << "Block Alloc Failed, disk space inadequate." << std::endl;
        return false;
    }//if
    auto blockdat = bprint_str(attr);
    env.m_fscore->blockwrite(block, blockdat);
    env.m_cur_inode->add_block(env, block);

    return false;
}//fs_create(env, nodename, owner_uid, flag)

bool
fs_delete(Env & env, const std::string & nodename);

std::pair<bool, std::vector<char>>
fs_read(Env & env, int32_t fd, uint64_t size);

bool
fs_write(Env & env, int32_t fd, const std::vector<char> & data);

int32_t
fs_open(Env & env, const std::string & filename, const AttrFlag & flag);

bool
fs_close(int32_t fd);

bool
fs_init(const std::string & fs_filename, int32_t fs_size, int16_t fs_block_size) {
    if(fs_size % fs_block_size > 0) {
        std::cerr << "fs_size must be a multiple of fs_block_size" << std::endl;
        std::abort();
    }//if

    std::ofstream ofs(fs_filename, std::ofstream::binary);
    if(!ofs.is_open()) {
        std::cerr << "fs_init error: file not found: " << fs_filename << std::endl;
        return false;
    }//if

    FSInfo info;
    {
        ofs.seekp(0, std::ios_base::beg);
        info.m_fs_size = fs_size;
        info.m_fs_block_sz = fs_block_size;

        // write FSInfo at the beginning as a block
        // * BlockInfo (16 bits, 2 bytes)
        //  * bit16: the number of bytes used
        uint16_t info_sz = FSInfo::c_struct_sz;
        ofs.write((char *)&info_sz, sizeof(uint16_t));

        auto info_binary = bprint_str(info);
        ofs.write(info_binary.data(), info_binary.size());
        ofs.flush();
    }//plain block

    // write '/' root dir attributes
    Attribute rootattr;
    {
        ofs.seekp(1 * fs_block_size, std::ios_base::beg);
        rootattr.m_flag = AttrFlag(AttrFlag::afs_dir | AttrFlag::afs_read | AttrFlag::afs_write);
        rootattr.m_owner_uid = int8_t(0); // root's uid is 0
        rootattr.m_inode = std::make_shared<INode>();
        rootattr.m_inode->m_blocks_num = 0;
        rootattr.m_node_name = "/";

        // * BlockInfo (16 bits, 2 bytes)
        //  * bit16: the number of bytes used
        uint16_t rootattr_sz = Attribute::c_struct_sz;
        ofs.write((char *)&rootattr_sz, sizeof(uint16_t));

        auto rootattr_binary = bprint_str(rootattr);
        ofs.write(rootattr_binary.data(), rootattr_binary.size());
        ofs.flush();
    }//plain block

    // reserve the disk space for the filesystem
    {
        ofs.seekp(fs_size, std::ios_base::beg);
        int32_t dummy = 0;
        ofs.write((char *)&dummy, sizeof(int32_t));
        ofs.flush();
    }//plain block

    // make a fake env
    Env fake_env;
    {
        fake_env.m_cur_uid = 0; // root's uid is 0
        fake_env.m_fscore = std::make_shared<FSCore>(fs_filename);
        fake_env.m_cur_path = "/";
        fake_env.m_cur_inode = rootattr.m_inode;
    }//plain block

    // create "/etc/", "/home/", "/tmp/" directories
    {
        AttrFlag flag = (AttrFlag)(AttrFlag::afs_dir | AttrFlag::afs_read | AttrFlag::afs_write);
        fs_create(fake_env, "etc", fake_env.m_cur_uid, flag);
        fs_create(fake_env, "home", fake_env.m_cur_uid, flag);
        fs_create(fake_env, "tmp", fake_env.m_cur_uid, flag);
    }//plain block

    ofs.close();

    return true;
}//fs_init(fs_filename, fs_size, fs_block_sz)

}//namespace afs

