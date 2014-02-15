#pragma once

// STL
#include <string>
#include <memory>

namespace afs {

struct INode;
class FSCore;

/**
 * @brief Env saves current infomation that all interfaces need
 * @param m_cur_uid: current logged-in user id
 * @param m_cur_path: current absolute path
 * @param m_attr_addr: current directory Attribute block address
 * @param m_fscore: an FSCore pointer that provides all core functions
 */
struct Env {
    int m_cur_uid;
    std::string m_cur_path;
    int16_t m_attr_addr;
    std::shared_ptr<FSCore> m_fscore;
};//struct Env

}//namespace afs

