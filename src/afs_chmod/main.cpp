#include "afs_chmod/afs_chmod.hpp"
// STL
#include <iostream>
#include <string>
// Boost
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
// afs
#include "afs/all.hpp"

int main(int argc, char * argv[]) {
    if(argc != 6) {
        printHelp();
        return EXIT_FAILURE;
    }//if

    auto all_env = afs::init_env(argv[1], argv[2]);

    std::string path(argv[3]);
    std::string new_owner(argv[4]);
    afs::AttrFlag flag = static_cast<afs::AttrFlag>(boost::lexical_cast<int>(argv[5]));

    // if it is a directory, check whether flag contains dir flag
    boost::trim(path);
    if(path.back() == '/') {
        if(!afs::isDir(flag)) {
            std::cerr << "afs_chmod failed, cannot change mode of a directory"
                << " without a is_directory flag" << std::endl;
            return EXIT_FAILURE;
        }//if
        path.pop_back();
    }//if

    // check new owner is a valid user
    auto new_owner_iter = all_env.second.first.find(new_owner);
    if(new_owner_iter == all_env.second.first.end()) {
        std::cerr << "afs_chmod failed, new owner is not a valid user" << std::endl;
        return EXIT_FAILURE;
    }//if
    int new_owner_uid = new_owner_iter->second;

    // split the filename and the parent directory path
    auto split_filename_pos = path.find_last_of('/');
    std::string parent_path(path.begin(), path.begin() + split_filename_pos);
    std::string filename(path.begin() + split_filename_pos + 1, path.end());

    // cd into directory before changing node's mode
    if(!afs::fs_change_directory(all_env.first, parent_path)) {
        std::cerr << "afs_chmod failed, path not valid." << std::endl;
        return EXIT_FAILURE;
    }//if

    // find the file in current directory
    auto nodelist = afs::fs_list_directory(all_env.first);
    for(auto && i : nodelist) {
        if(i.second->m_node_name == filename) {
            // check dirflag
            if(afs::isDir(i.second->m_flag) && !afs::isDir(flag)) {
                std::cerr << "afs_chmod failed, cannot change a directory into file" << std::endl;
                return EXIT_FAILURE;
            }//if
            if(!afs::isDir(i.second->m_flag) && afs::isDir(flag)) {
                std::cerr << "afs_chmod failed, cannot change a file into directory" << std::endl;
                return EXIT_FAILURE;
            }//if

            // check uid for permission
            if(i.second->m_owner_uid != all_env.first.m_cur_uid) {
                std::cerr << "afs_chmod failed, do not have the permission to chmod file, "
                    << "probably you may change a user to login." << std::endl;
                return EXIT_FAILURE;
            }//if

            i.second->m_owner_uid = new_owner_uid;
            i.second->m_flag = flag;

            // write back
            auto attrdat = bprint_str(*i.second);
            all_env.first.m_fscore->blockwrite(i.first, attrdat);
            return 0;
        }//if
    }//for

    return EXIT_FAILURE;
}//main

void
printHelp() {
    std::cerr << "usage: afs_chmod [afs_fsfile] [user] [file_name] [owner] [file_flag_number]"
        << std::endl;
    std::cerr << "file_flag_number:" << std::endl;
    std::cerr << "  1 for it is a directory" << std::endl;
    std::cerr << "  2 for it is readable" << std::endl;
    std::cerr << "  4 for it is writable" << std::endl;
    std::cerr << "  and combinations of them, such as 6 indicating readable and writable, " << std::endl;
    std::cerr << "  are also available." << std::endl;
    std::cerr << "note: if the file was not a directory and you want to change it into directory,"
        << std::endl;
    std::cerr << "  you should delete the file and recreate a directory." << std::endl;
    std::cerr << "example: afs_chmod ./myfs.fs root /tmp/testfile guest 4" << std::endl;
    std::cerr << "example: afs_chmod ./myfs.fs root /tmp/testfile root 2" << std::endl;
}//printHelp()


