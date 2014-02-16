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
INode::add_block(const Env & env, int16_t blockid) {
    // 1-level-index
    int16_t i = 0;
    for(i = 0; i < 10; ++i) {
        if(m_addr[i] == 0) {
            m_addr[i] = blockid;
            ++m_blocks_num;
            return true;
        }//if
    }//for

    const std::size_t addrnums = env.m_fscore->fs_data_max_sz()/sizeof(int16_t);
    // 2-level-index
    for(i = 10; i < c_addrnum; ++i) {
        if(m_addr[i] == 0) {
            int16_t block = alloc_one_block(env);
            if(block == -1)
                return false;

            std::vector<char> blockdat(env.m_fscore->fs_data_max_sz(), 0);
            *(int16_t *)(blockdat.data()) = blockid;
            env.m_fscore->blockwrite(block, blockdat);

            m_addr[i] = block;
            ++m_blocks_num;
            return true;
        } else {
            auto level1 = env.m_fscore->blockread(m_addr[i], 1);
            level1.resize(env.m_fscore->fs_data_max_sz());
            int16_t * pint16_t = (int16_t *)level1.data();
            for(std::size_t j = 0; j < addrnums; ++j) {
                if(pint16_t[j] == 0) {
                    pint16_t[j] = blockid;
                    env.m_fscore->blockwrite(m_addr[i], level1);
                    ++m_blocks_num;
                    return true;
                }//if
            }//for
        }//if-else
    }//for

    return false;
}//add_block(blockid)

void
INode::format(const Env & env) {
    // 1-level-index
    int16_t i = 0;
    for(i = 0; i < 10; ++i) {
        if(m_addr[i] == 0) {
            return;
        } else {
            env.m_fscore->blockformat(m_addr[i]);
            m_addr[i] = 0;
        }//if-else
    }//for

    const std::size_t addrnums = env.m_fscore->fs_data_max_sz()/sizeof(int16_t);
    // 2-level-index
    for(i = 10; i < c_addrnum; ++i) {
        if(m_addr[i] == 0) {
            return;
        } else {
            auto level1 = env.m_fscore->blockread(m_addr[i], 1);
            int16_t * pint16_t = (int16_t *)level1.data();
            for(std::size_t j = 0; j < addrnums; ++j) {
                if(pint16_t[j] == 0) {
                    break;
                } else {
                    env.m_fscore->blockformat(pint16_t[j]);
                    pint16_t[j] = 0;
                }//if-else
            }//for

            env.m_fscore->blockformat(m_addr[i]);
            m_addr[i] = 0;
        }//if-else
    }//for
}//format()

}//namespace afs

