#include "ghelper.hpp"
// STL
#include <iostream>
#include <string>
// Boost
#include <boost/tokenizer.hpp>
// afs
#include "afs/env/env.hpp"
#include "afs/fsinterface/fsinterface.hpp"

namespace afs {

std::pair<
    std::unordered_map<std::string, std::size_t>,
    std::unordered_map<std::size_t, std::string>>
load_user_uid_map(const std::shared_ptr<FSCore> & fscore) {
    std::pair<
        std::unordered_map<std::string, std::size_t>,
        std::unordered_map<std::size_t, std::string>> ret;

    Env fake_env;
    fake_env.m_cur_uid = 0; // 0 for root
    fake_env.m_cur_path = "/";
    fake_env.m_attr_addr = 1;
    fake_env.m_fscore = fscore;

    if(!fs_change_directory(fake_env, "/etc")) {
        std::cerr << "load_user_uid_map, cd to /etc/ failed" << std::endl;
        std::abort();
    }//if

    auto readdat = fs_read(fake_env, "passwd");
    if(readdat.first != 1)
        std::cerr << "read from /etc/passwd failed" << std::endl;
    std::string passwdstr(readdat.second.begin(), readdat.second.end());

    boost::char_separator<char> sep(":\n");
    boost::tokenizer<boost::char_separator<char>> toker(passwdstr, sep);

    auto iter = toker.begin(), iter_end = toker.end();
    while(iter != iter_end) {
        std::string username = *iter++;
        if(iter == iter_end) {
            std::cerr << "Corrupted /etc/passwd, load users failed!" << std::endl;
            std::abort();
        }//if
        std::size_t uid = std::stoi(*iter++);

        ret.first.insert(std::make_pair(username, uid));
        ret.second.insert(std::make_pair(uid, username));
    }//while
    return ret;
}//load_user_uid_map(fscore)

std::pair<
    Env,
    std::pair<
        std::unordered_map<std::string, std::size_t>,
        std::unordered_map<std::size_t, std::string>>
>
init_env(const std::string & fsfile, const std::string username) {
    std::shared_ptr<afs::FSCore> fscore;
    try {
        fscore = std::make_shared<afs::FSCore>(fsfile);
    } catch(std::runtime_error & e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }//try-catch
    
    afs::Env env;
    auto usermaps = afs::load_user_uid_map(fscore);
    {
        // init current uid
        auto iter = usermaps.first.find(username);
        if(iter == usermaps.first.end()) {
            std::cerr << "Invalid username: " << username << std::endl;
            std::exit(EXIT_FAILURE);
        }//if
        env.m_cur_uid = iter->second;
    }//plain block
    env.m_cur_path = "/";
    env.m_attr_addr = 1;
    env.m_fscore = fscore;
    return std::make_pair(std::move(env), std::move(usermaps));
}//init_env(fsfile, username)

}//namespace afs

