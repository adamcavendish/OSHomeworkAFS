#include "afs/inode/inode.hpp"
// STL
#include <iostream>
#include <memory>
#include <functional>
// afs
#include "afs/env/env.hpp"
#include "afs/fsinterface/fsinterface_helper.hpp"

namespace afs {

bool
INode::add_block(Env & env, int16_t blockid) {
    // 1-level-index
    int16_t i = 0;
    for(i = 0; i < 10; ++i) {
        if(m_addr[i] == 0) {
            m_addr[i] = blockid;
            ++m_blocks_num;
            return true;
        }//if
    }//for

    // 2-level-index
    for(i = 10; i < c_addrnum; ++i) {
        if(m_addr[i] == 0) {
            int16_t block = alloc_one_block(env);
            if(block == -1) {
                std::cerr << "Block Alloc Failed, disk space inadequate." << std::endl;
                return false;
            }//if
            std::vector<char> blockdat(env.m_fscore->fs_data_max_sz(), 0);
            *(int16_t *)(blockdat.data()) = blockid;
            env.m_fscore->blockwrite(block, blockdat);

            ++m_blocks_num;
            return true;
        } else {
            const std::size_t addrnums = env.m_fscore->fs_data_max_sz()/sizeof(int16_t);

            auto level1 = env.m_fscore->blockread(m_addr[i], 1);
            int16_t * pint16_t = (int16_t *)level1.data();
            for(std::size_t i = 0; i < addrnums; ++i) {
                if(pint16_t[i] == 0) {
                    pint16_t[i] = blockid;
                    ++m_blocks_num;
                    return true;
                }//if
            }//for
        }//if-else
    }//for

    return false;
}//add_block(blockid)

}//namespace afs

