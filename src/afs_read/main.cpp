#include "afs_read/afs_read.hpp"
// STL
#include <iostream>
#include <string>
// Boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
// afs
#include "afs/all.hpp"

int main(int argc, char * argv[]) {
    if(argc != 4 && argc != 5) {
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

    // cd into directory before read the file
    if(!afs::fs_change_directory(all_env.first, parent_path)) {
        std::cerr << "afs_rm failed, path not valid." << std::endl;
        return EXIT_FAILURE;
    }//if

    std::pair<int, std::vector<char>> readret;
    if(argc == 5) {
        std::size_t readin_size = 0;
        readin_size = boost::lexical_cast<std::size_t>(argv[4]);
        readret = afs::fs_read(all_env.first, filename, readin_size);
    } else {
        readret = afs::fs_read(all_env.first, filename);
    }//if-else

    switch(readret.first) {
    case -1:
        std::cerr << "file does not exist!" << std::endl;
        return EXIT_FAILURE;
    case -2:
        std::cerr << "cannot use fs_read to read-in a directory!" << std::endl;
        return EXIT_FAILURE;
    case -3:
        std::cerr << "file is not granted readable permission" << std::endl;
        return EXIT_FAILURE;
    default:
        break;
    }//switch-case

    for(auto && i : readret.second) {
        std::cout << i;
    }//for

    return 0;
}//main

void
printHelp() {
    std::cerr << "usage: afs_read [afs_fsfile] [user] [file_name] ([size])" << std::endl;
    std::cerr << "example: afs_read ./myfs.fs root /tmp/testfile" << std::endl;
    std::cerr << "example: afs_read ./myfs.fs root /tmp/testfile 1000" << std::endl;
}//printHelp()

