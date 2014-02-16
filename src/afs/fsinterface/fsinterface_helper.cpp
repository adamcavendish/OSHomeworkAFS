#include "afs/fsinterface/fsinterface_helper.hpp"
// afs
#include "afs/env/env.hpp"
#include "afs/inode/inode.hpp"
#include "afs/fscore/fscore.hpp"

namespace afs {

int16_t
alloc_one_block(const Env & env) {
    auto fs_sz = env.m_fscore->fs_size();
    auto fs_block_sz = env.m_fscore->fs_block_sz();

    auto blocknums = fs_sz/fs_block_sz;
    for(int16_t i = 1; i < blocknums; ++i) {
        if(env.m_fscore->blockused(i) == false)
            return i;
    }//for

    return -1;
}//alloc_one_block(env)
    
}//namespace afs

