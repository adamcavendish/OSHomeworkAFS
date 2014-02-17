#include "afs_ls/afs_ls.hpp"
// STL
#include <iostream>
#include <memory>
// C-STL
#include <cstdlib>
// afs
#include "afs/all.hpp"

int main(int argc, char * argv[]) {
    if(argc != 4) {
        printHelp();
        return EXIT_FAILURE;
    }//if

    auto all_env = afs::init_env(argv[1], argv[2]);

    std::string path(argv[3]);
    if(path[0] != '/') {
        std::cerr << "afs_ls accepts only absolute paths" << std::endl;
        return EXIT_FAILURE;
    }//if

    if(!afs::fs_change_directory(all_env.first, path)) {
        std::cerr << "afs_ls incorrect path" << std::endl;
        return EXIT_FAILURE;
    }//if

    auto nodelist = afs::fs_list_directory(all_env.first);
    for(auto && i : nodelist) {
        std::cout << printFlag(i.second->m_flag) << " ";
        std::cout << all_env.second.second[i.second->m_owner_uid] << " ";
        std::cout << i.second->m_node_name << std::endl;
    }//for

    return 0;
}//main

void
printHelp() {
    std::cerr << "usage: afs_ls [afs_fsfile] [user] [path]" << std::endl;
    std::cerr << "example: afs_ls ./myfs.fs root /tmp/testfile.txt" << std::endl;
}//printHelp()

std::string
printFlag(const afs::AttrFlag & flag) {
    std::string ret;
    
    if(afs::isDir(flag)) {
        ret += 'd';
    } else {
        ret += '-';
    }//if-else

    if(afs::isReadable(flag)) {
        ret += 'r';
    } else {
        ret += '-';
    }//if-else

    if(afs::isWritable(flag)) {
        ret += 'w';
    } else {
        ret += '-';
    }//if-else

    return ret;
}//printFlag(flag)

