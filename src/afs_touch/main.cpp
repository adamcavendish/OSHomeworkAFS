#include "afs_touch/afs_touch.hpp"
// STL
#include <iostream>
#include <memory>
// C-STL
#include <cstdlib>
// Boost
#include <boost/algorithm/string.hpp>
// afs
#include "afs/all.hpp"

int main(int argc, char * argv[]) {
    if(argc != 5) {
        printHelp();
        return EXIT_FAILURE;
    }//if

    auto all_env = afs::init_env(argv[1], argv[2]);

    std::string path(argv[3]);
    afs::AttrFlag flag = static_cast<afs::AttrFlag>(std::stoi(argv[4]));

    // if it is a directory, check whether flag contains dir flag
    boost::trim(path);
    if(path.back() == '/') {
        if(!afs::isDir(flag)) {
            std::cerr << "afs_touch failed, cannot create a directory without a is_directory flag"
                << std::endl;
            return EXIT_FAILURE;
        }//if
        path.pop_back();
    }//if

    // split the filename and the parent directory path
    auto split_filename_pos = path.find_last_of('/');
    std::string parent_path(path.begin(), path.begin() + split_filename_pos);
    std::string filename(path.begin() + split_filename_pos + 1, path.end());

    // cd into directory before creation
    if(!afs::fs_change_directory(all_env.first, parent_path)) {
        std::cerr << "afs_touch failed, path not valid." << std::endl;
        return EXIT_FAILURE;
    }//if
    
    auto create_code = afs::fs_create(all_env.first, filename, all_env.first.m_cur_uid, flag);
    switch(create_code) {
    case -1:
        std::cerr << "afs_touch failed, file already exist." << std::endl;
        return EXIT_FAILURE;
    case -2:
        std::cerr << "afs_touch failed, filesystem inadequate space" << std::endl;
        return EXIT_FAILURE;
    case -3:
        std::cerr << "afs_touch failed, files/dirs reaches upper limit" << std::endl;
        return EXIT_FAILURE;
    default:
        break;
    }//switch-case

    return 0;
}//main

void
printHelp() {
    std::cerr << "usage: afs_touch [afs_fsfile] [user] [file_name] [file_flag_number]" << std::endl;
    std::cerr << "file_flag_number:" << std::endl;
    std::cerr << "  1 for it is a directory" << std::endl;
    std::cerr << "  2 for it is readable" << std::endl;
    std::cerr << "  4 for it is writable" << std::endl;
    std::cerr << "  and combinations of them, such as 6 indicating readable and writable, " << std::endl;
    std::cerr << "  are also available." << std::endl;
    std::cerr << "example: afs_touch ./myfs.fs root /tmp/testfile.txt 6" << std::endl;
    std::cerr << "example: afs_touch ./myfs.fs root /tmp/mydir 7" << std::endl;
    std::cerr << "example: afs_touch ./myfs.fs root /tmp/mydir/ 7" << std::endl;
}//printHelp()

