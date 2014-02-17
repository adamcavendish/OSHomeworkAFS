#include "afs_rmuser/afs_rmuser.hpp"
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

    std::shared_ptr<afs::FSCore> fscore;
    try {
        fscore = std::make_shared<afs::FSCore>(argv[1]);
    } catch(std::runtime_error & e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }//try-catch

    // use a fake_env
    afs::Env fake_env;
    fake_env.m_cur_uid = 0; // 0 for root
    fake_env.m_cur_path = "/";
    fake_env.m_attr_addr = 1;
    fake_env.m_fscore = fscore;

    if(!fs_change_directory(fake_env, "/etc")) {
        std::cerr << "load_user_uid_map, cd to /etc/ failed" << std::endl;
        std::abort();
    }//if

    // read out /etc/passwd
    auto readdat = fs_read(fake_env, "passwd");
    if(readdat.first != 1)
        std::cerr << "read from /etc/passwd failed" << std::endl;
    std::string passwdstr(readdat.second.begin(), readdat.second.end());

    // get username to be removed
    std::string rm_username(argv[3]);

    // find username in the passwd file
    auto pos = passwdstr.find(rm_username);
    if(pos == std::string::npos) {
        std::cerr << "afs_rmuser failed: user not found." << std::endl;
        return EXIT_FAILURE;
    }//if

    // find `username:uid` line, and remove it
    auto pos_end = passwdstr.find_first_of('\n', pos);
    passwdstr.erase(pos, pos_end - pos + 1);

    // writeback all the config file
    std::vector<char> writedat(passwdstr.begin(), passwdstr.end());
    if(afs::fs_write(fake_env, "passwd", writedat) != 1) {
        std::cerr << "afs_rmuser failed: /etc/passwd write error" << std::endl;
        return EXIT_FAILURE;
    }//if

    return 0;
}//main

void
printHelp() {
    std::cerr << "usage: afs_rmuser [afs_fsfile] [user] [username]" << std::endl;
    std::cerr << "example: a" << std::endl;
}//printHelp()

