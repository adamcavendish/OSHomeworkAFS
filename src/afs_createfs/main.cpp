#include "afs_createfs/afs_createfs.hpp"
// STL
#include <iostream>
#include <string>
#include <memory>
// C-STL
#include <cstdlib>
// Boost
#include <boost/lexical_cast.hpp>
// afs
#include "afs/all.hpp"

int main(int argc, char * argv[]) {
    if(argc != 4) {
        printHelp();
        return EXIT_FAILURE;
    }//if

    std::string fsname(argv[1]);
    std::size_t fssize(boost::lexical_cast<std::size_t>(argv[2]));
    std::size_t fs_blocksz(boost::lexical_cast<std::size_t>(argv[3]));
    if(!afs::fs_init(fsname, fssize, fs_blocksz))
        return EXIT_FAILURE;

    return 0;
}//main

void
printHelp() {
    std::cerr << "usage: afs_createfs [afs_fsfile] [fs_size] [fs_block_size]" << std::endl;
    std::cerr << "note: 1. fs_size must be a multiple of fs_block_size" << std::endl;
    std::cerr << "      2. fs_block_size must be greater than 64" << std::endl;
    std::cerr << "example: afs_createfs ./myfs.fs 102400 64" << std::endl;
    std::cerr << "example: afs_createfs ./myfs.fs 102400 100" << std::endl;
}//printHelp()

