#pragma once

// STL
#include <string>
#include <memory>
// afs
#include "afs/fscore/fscore.hpp"
#include "afs/inode/inode.hpp"

namespace afs {

struct Env {
    int m_cur_uid;
    std::string m_cur_path;
    std::shared_ptr<INode> m_cur_inode;
    std::shared_ptr<FSCore> m_fscore;
};//struct Env

}//namespace afs

