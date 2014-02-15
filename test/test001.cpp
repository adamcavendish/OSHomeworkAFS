// STL
#include <iostream>
// gtest
#include <gtest/gtest.h>
// afs
#include "afs/fsinterface/fsinterface.hpp"
using namespace afs;

TEST(test, t001) {
    afs::fs_init("abc.fs", 10240, 64);

    FSCore fscore("abc.fs");
    ASSERT_EQ(fscore.fs_size(), 10240);
    ASSERT_EQ(fscore.fs_block_sz(), 64);

    std::vector<char> dat { 1, 3, 5, 7, 9 };
    fscore.blockwrite(10, dat);

    auto datread = fscore.blockread(10, 1);
    ASSERT_EQ(datread, dat);

    for(auto && i : datread) {
        std::cout << (int)i << ' ';
    }//for
    std::cout << std::endl;
}

TEST(test, t002) {
    afs::fs_init("abc.fs", 10240, 64);

    FSCore fscore("abc.fs");
    ASSERT_EQ(fscore.fs_size(), 10240);
    ASSERT_EQ(fscore.fs_block_sz(), 64);

    std::vector<char> datread;
    for(auto i = 0; i < 10; ++i) {
        datread = fscore.blockread(i, 1);
        std::cout << "block " << i << ": ";
        for(auto && i : datread) {
            std::cout << (int)i << ' ';
        }//for
        std::cout << std::endl;
    }//for
}

TEST(test, t003) {
    afs::fs_init("abc.fs", 10240, 64);

    FSCore fscore("abc.fs");
    ASSERT_EQ(fscore.fs_size(), 10240);
    ASSERT_EQ(fscore.fs_block_sz(), 64);

    bool used;

    std::cout << "sizeof(AttrFlag): " << sizeof(AttrFlag) << std::endl;
    
    for(auto i = 0; i < 20; ++i) {
        used = fscore.blockused(i);
        std::cout << i << ": " << std::boolalpha << used << std::endl;
    }//for
}

TEST(test, t004) {
    afs::fs_init("abc.fs", 10240, 64);

    FSCore fscore("abc.fs");
    ASSERT_EQ(fscore.fs_size(), 10240);
    ASSERT_EQ(fscore.fs_block_sz(), 64);

    Attribute rootattr;
    rootattr.m_flag = AttrFlag(AttrFlag::afs_dir | AttrFlag::afs_read | AttrFlag::afs_write);
    rootattr.m_owner_uid = int8_t(0); // root's uid is 0
    rootattr.m_inode = std::make_shared<INode>();
    rootattr.m_inode->m_blocks_num = 0;
    rootattr.m_node_name = "/";

    auto dat = bprint_str(rootattr);

    Attribute readout;
    bscan_str(readout, dat.begin(), dat.end());

    ASSERT_EQ(rootattr.m_flag, readout.m_flag);
    ASSERT_EQ(rootattr.m_owner_uid, readout.m_owner_uid);
    ASSERT_EQ(rootattr.m_inode->m_blocks_num, readout.m_inode->m_blocks_num);
    for(auto i = 0; i < INode::c_addrnum; ++i)
        ASSERT_EQ(rootattr.m_inode->m_addr[i], readout.m_inode->m_addr[i]);
    ASSERT_EQ(rootattr.m_node_name, readout.m_node_name);
}

TEST(test, t005) {
    afs::fs_init("abc.fs", 10240, 64);

    auto fscore = std::make_shared<FSCore>("abc.fs");
    ASSERT_EQ(fscore->fs_size(), 10240);
    ASSERT_EQ(fscore->fs_block_sz(), 64);

    auto rootattr = std::make_shared<Attribute>();
    auto rootdat = fscore->blockread(1, 1);
    if(!bscan_str(*rootattr, rootdat.begin(), rootdat.end()))
        FAIL();

    Env env;
    env.m_cur_uid = 0;
    env.m_cur_path = "/";
    env.m_attr_addr = 1;
    env.m_fscore = fscore;

    std::cout << "list / files and directories: " << std::endl;
    auto attrs = fs_list_directory(env);
    for(auto && i : attrs)
        std::cout << i->m_node_name << std::endl;
}

int main(int argc, char * argv[])
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}//main

