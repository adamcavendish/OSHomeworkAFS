#pragma once

// STL
#include <vector>
// afs
#include "afs/attribute/attribute.hpp"
#include "afs/inode/inode.hpp"

namespace afs {

struct DirAttr {
    Attribute m_attr;
    INode m_subdir_info;
};//struct DirAttr

}//namespace afs
