#include "afs_adduser/afs_adduser.hpp"
// STL
#include <iostream>
#include <string>
// afs
#include "afs/all.hpp"

int main(int argc, char * argv[]) {
    if(argc != 4) {
        printHelp();
        return EXIT_FAILURE;
    }//if

    auto all_env = afs::init_env(argv[1], argv[2]);

    // check new username is valid or not
    std::string newuser(argv[3]);
    auto iter1 = all_env.second.first.find(newuser);
    if(iter1 != all_env.second.first.end()) {
        std::cerr << "afs_adduser failed: user already exists." << std::endl;
        return EXIT_FAILURE;
    }//if

    // allocate a new uid
    auto iter2 = std::prev(all_env.second.second.end());
    std::size_t newuid = iter2->first + 1;

    // cd into /etc
    if(!afs::fs_change_directory(all_env.first, "/etc")) {
        std::cerr << "afs_adduser failed: /etc not found." << std::endl;
        return EXIT_FAILURE;
    }//if

    // readout /etc/passwd
    auto readret = afs::fs_read(all_env.first, "passwd");
    if(readret.first != 1) {
        std::cerr << "afs_adduser failed: /etc/passwd readin error" << std::endl;
        return EXIT_FAILURE;
    }//if

    // build `newuser:newuid` format string
    std::string tmp;
    tmp += newuser;
    tmp += ":";
    tmp += std::to_string(newuid);
    tmp += '\n';

    // insert the newuser format string to the back
    readret.second.insert(readret.second.end(), tmp.begin(), tmp.end());
    
    // Escalation
    all_env.first.m_cur_uid = 0; // 0 for root
    // writeback all the config file
    if(afs::fs_write(all_env.first, "passwd", readret.second) != 1) {
        std::cerr << "afs_adduser failed: /etc/passwd write error" << std::endl;
        return EXIT_FAILURE;
    }//if

    return 0;
}//main

void
printHelp() {
    std::cerr << "usage: afs_adduser [afs_file] [user] [username]" << std::endl;
    std::cerr << "Note: [user] is the login user, [username] is the new user to be added"
        << std::endl;
    std::cerr << "example: afs_adduser ./myfs.fs root newuser" << std::endl;
    std::cerr << "example: afs_adduser ./myfs.fs root foouser" << std::endl;
}//printHelp()



