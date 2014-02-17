#include "afs_rm/afs_rm.hpp"
// STL
#include <iostream>
#include <string>
// Boost
#include <boost/algorithm/string.hpp>
// afs
#include "afs/all.hpp"

int main(int argc, char * argv[]) {
    if(argc != 4) {
        printHelp();
        return EXIT_FAILURE;
    }//if

    auto all_env = afs::init_env(argv[1], argv[2]);

    std::string path(argv[3]);
    boost::trim(path);
    if(path.back() == '/')
        path.pop_back();

    // split the filename and the parent directory path
    auto split_filename_pos = path.find_last_of('/');
    std::string parent_path(path.begin(), path.begin() + split_filename_pos);
    std::string filename(path.begin() + split_filename_pos + 1, path.end());

    // cd into directory before remove
    if(!afs::fs_change_directory(all_env.first, parent_path)) {
        std::cerr << "afs_rm failed, path not valid." << std::endl;
        return EXIT_FAILURE;
    }//if

    auto deletecode = afs::fs_delete(all_env.first, filename);
    if(deletecode == -1) {
        std::cerr << "afs_rm failed, file not found." << std::endl;
        return EXIT_FAILURE;
    }//if

    return 0;
}//main

void
printHelp() {
    std::cerr << "usage: afs_rm [afs_fsfile] [user] [file_name]" << std::endl;
    std::cerr << "example: afs_rm ./myfs.fs root /tmp/testfile" << std::endl;
}//printHelp()

