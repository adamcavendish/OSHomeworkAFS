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
    ASSERT_EQ(fscore.m_size, 10240);
    ASSERT_EQ(fscore.m_block_sz, 64);

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
    ASSERT_EQ(fscore.m_size, 10240);
    ASSERT_EQ(fscore.m_block_sz, 64);

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
    ASSERT_EQ(fscore.m_size, 10240);
    ASSERT_EQ(fscore.m_block_sz, 64);

    bool used;

    std::cout << "sizeof(AttrFlag): " << sizeof(AttrFlag) << std::endl;
    
    for(auto i = 0; i < 20; ++i) {
        used = fscore.blockused(i);
        std::cout << i << ": " << std::boolalpha << used << std::endl;
    }//for
}

int main(int argc, char * argv[])
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}//main

