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

    auto datread = fscore.blockread(1, 1);

    for(auto && i : datread) {
        std::cout << (int)i << ' ';
    }//for
    std::cout << std::endl;
}

int main(int argc, char * argv[])
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}//main

