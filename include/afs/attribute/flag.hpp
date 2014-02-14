#pragma once
// C-STL
#include <cstdint>

namespace afs {

enum AttrFlag : int8_t {
    afs_dir         = 0x01,
    afs_read        = 0x02,
    afs_write       = 0x04,
};//enum AttrFlag

inline bool
isDir(AttrFlag flag)
{ return flag & afs_dir; }

inline bool
isReadable(AttrFlag flag) 
{ return flag & afs_read; }

inline bool
isWritable(AttrFlag flag)
{ return flag & afs_write; }

}//namespace afs

