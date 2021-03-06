// STL
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
// Boost
#include <boost/lexical_cast.hpp>
// gtest
#include <gtest/gtest.h>
// afs
#include "afs/fsinterface/fsinterface.hpp"
using namespace afs;

int * gargc;
char ** gargv;

std::size_t gfs_size = 10240;
std::size_t gfs_blocksz = 64;

TEST(test, t001) {
    afs::fs_init("abc.fs", gfs_size, gfs_blocksz);

    FSCore fscore("abc.fs");
    ASSERT_EQ(fscore.fs_size(), gfs_size);
    ASSERT_EQ(fscore.fs_block_sz(), gfs_blocksz);

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
    afs::fs_init("abc.fs", gfs_size, gfs_blocksz);

    FSCore fscore("abc.fs");
    ASSERT_EQ(fscore.fs_size(), gfs_size);
    ASSERT_EQ(fscore.fs_block_sz(), gfs_blocksz);

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
    afs::fs_init("abc.fs", gfs_size, gfs_blocksz);

    FSCore fscore("abc.fs");
    ASSERT_EQ(fscore.fs_size(), gfs_size);
    ASSERT_EQ(fscore.fs_block_sz(), gfs_blocksz);

    bool used;

    std::cout << "sizeof(AttrFlag): " << sizeof(AttrFlag) << std::endl;
    
    for(auto i = 0; i < 20; ++i) {
        used = fscore.blockused(i);
        std::cout << i << ": " << std::boolalpha << used << std::endl;
    }//for
}

TEST(test, t004) {
    afs::fs_init("abc.fs", gfs_size, gfs_blocksz);

    FSCore fscore("abc.fs");
    ASSERT_EQ(fscore.fs_size(), gfs_size);
    ASSERT_EQ(fscore.fs_block_sz(), gfs_blocksz);

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
    afs::fs_init("abc.fs", gfs_size, gfs_blocksz);

    auto fscore = std::make_shared<FSCore>("abc.fs");
    ASSERT_EQ(fscore->fs_size(), gfs_size);
    ASSERT_EQ(fscore->fs_block_sz(), gfs_blocksz);

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
        std::cout << i.second->m_node_name << std::endl;
}

TEST(test, t006) {
    afs::fs_init("abc.fs", gfs_size, gfs_blocksz);

    auto fscore = std::make_shared<FSCore>("abc.fs");
    ASSERT_EQ(fscore->fs_size(), gfs_size);
    ASSERT_EQ(fscore->fs_block_sz(), gfs_blocksz);

    Env env;
    env.m_cur_uid = 0;
    env.m_cur_path = "/";
    env.m_attr_addr = 1;
    env.m_fscore = fscore;

    ASSERT_EQ(fs_change_directory(env, "/etc/"), true);
    ASSERT_EQ(fs_change_directory(env, "/etc/abc/def"), false);
}

TEST(test, t007) {
    afs::fs_init("abc.fs", gfs_size, gfs_blocksz);

    auto fscore = std::make_shared<FSCore>("abc.fs");
    ASSERT_EQ(fscore->fs_size(), gfs_size);
    ASSERT_EQ(fscore->fs_block_sz(), gfs_blocksz);

    Env env;
    env.m_cur_uid = 0;
    env.m_cur_path = "/";
    env.m_attr_addr = 1;
    env.m_fscore = fscore;

    std::vector<char> dat;
    {
        std::ifstream ifs("Makefile");
        if(!ifs.is_open()) {
            std::cout << "Opening file Makefile error." << std::endl;
            FAIL();
        }//if
        dat.insert(dat.end(), std::istreambuf_iterator<char>(ifs),
                std::istreambuf_iterator<char>());
        ifs.close();

        if(dat.size() > 1000)
            dat.resize(1000);
    }//plain block

    if(!fs_change_directory(env, "/tmp")) {
        std::cerr << "fs_change_directory fails" << std::endl;
        FAIL();
    }//if

    if(!fs_create(env, "testfile", 0, (AttrFlag)(AttrFlag::afs_read | AttrFlag::afs_write))) {
        std::cerr << "fs_create fails" << std::endl;
        FAIL();
    }//if

    std::cout << "writein size: " << dat.size() << std::endl;
    auto writecode = fs_write(env, "testfile", dat);
    if(writecode != 1) {
        std::cerr << "fs_write fails, code: " << writecode << std::endl;
        FAIL();
    }//if
    auto readstr = fs_read(env, "testfile");
    if(readstr.first != 1) {
        std::cerr << "fs_read fails, code: " << readstr.first << std::endl;
        FAIL();
    }//if
    std::cout << "readout size: " << readstr.second.size() << std::endl;
    for(auto && i : readstr.second) {
        std::cout << i;
    }//for
    std::cout << std::endl;

    ASSERT_EQ(readstr.second, dat);

    if(fs_delete(env, "testfile") != 1) {
        std::cerr << "fs_delete fails" << std::endl;
        FAIL();
    }//if
}

TEST(test, t008) {
    afs::fs_init("abc.fs", gfs_size, gfs_blocksz);

    auto fscore = std::make_shared<FSCore>("abc.fs");
    ASSERT_EQ(fscore->fs_size(), gfs_size);
    ASSERT_EQ(fscore->fs_block_sz(), gfs_blocksz);

    auto usermaps = load_user_uid_map(fscore);
    for(auto i : usermaps.first)
        std::cout << "user: " << i.first << ", uid: " << i.second << std::endl;
}

int main(int argc, char * argv[])
{
	testing::InitGoogleTest(&argc, argv);
    gargc = &argc;
    gargv = argv;

    if(argc == 3) {
        gfs_size = boost::lexical_cast<std::size_t>(argv[1]);
        gfs_blocksz = boost::lexical_cast<std::size_t>(argv[2]);
    }//if

	return RUN_ALL_TESTS();
}//main

