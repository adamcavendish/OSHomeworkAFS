#include "afs/fsinterface/fsinterface.hpp"
// STL
#include <fstream>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <memory>
// C-STL
#include <cstdio>
// Boost
#include <boost/tokenizer.hpp>
// afs
#include "ghelper.hpp"
#include "afs/attribute/attribute.hpp"
#include "afs/attribute/flag.hpp"
#include "afs/fsinfo/fsinfo.hpp"
#include "afs/inode/inode.hpp"
#include "afs/fsinterface/fsinterface_helper.hpp"

namespace afs {

/**
 * @brief create a file/directory depending on the flag(dir or not)
 *
 * @return on success, return 1
 *      failure on there's a file/dir with the same name, return -1
 *      failure on inadequate space, return -2
 *      failure on number of files/dirs reaches upper limit, return -3
 */
int
fs_create(Env & env, const std::string & nodename, int8_t owner_uid, const AttrFlag & flag) {
    auto nodelist = fs_list_directory(env);
    for(auto && i : nodelist) {
        if(i.second->m_node_name == nodename) {
            return -1;
        }//if
    }//for

    if(nodename.find('/', 0) != std::string::npos) {
        std::cerr << "fs_create should not create a file/dir by a absolute path" << std::endl;
        std::abort();
    }//if

    auto parent_dat = env.m_fscore->blockread(env.m_attr_addr, 1);
    Attribute parent_attr;
    if(!bscan_str(parent_attr, parent_dat.begin(), parent_dat.end())) {
        std::cerr << "fs_create: parent directory Attribute load error!" << std::endl;
        std::abort();
    }//if

    Attribute subnode_attr;
    subnode_attr.m_flag = flag;
    subnode_attr.m_owner_uid = owner_uid;
    subnode_attr.m_node_name = nodename;
    subnode_attr.m_inode = std::make_shared<INode>();

    auto block = alloc_one_block(env);
    if(block == -1) {
        std::cerr << "Block Alloc Failed, disk space inadequate." << std::endl;
        return -2;
    }//if
    auto blockdat = bprint_str(subnode_attr);
    env.m_fscore->blockwrite(block, blockdat);

    if((parent_attr.m_inode->add_block(env, block)) == false) {
        std::cerr << "INode::add_block failed. Directories/Files reaches upper limit."
            << std::endl;
        // cleanup
        env.m_fscore->blockformat(block);
        return -3;
    }//if

    parent_dat = bprint_str(parent_attr);
    env.m_fscore->blockwrite(env.m_attr_addr, parent_dat);
    return true;
}//fs_create(env, nodename, owner_uid, flag)

/**
 * @brief delete a file/directory
 *
 * @return on success, return 1 
 *      on failure nodename not found, return -1
 */
int
fs_delete(Env & env, const std::string & nodename) {
    /**
     * @brief check whether node is `nodename`, if it is, format it
     * @return on success return true; on not found, return false
     */
    auto check_then_format_node = [&](int16_t addr) -> bool {
        Attribute nodeattr;
        auto attrdat = env.m_fscore->blockread(addr, 1);
        if(!bscan_str(nodeattr, attrdat.begin(), attrdat.end())) {
            std::cerr << "fs_delete: node Attribute load error!" << std::endl;
            std::abort();
        }//if
        if(nodeattr.m_node_name == nodename) {
            nodeattr.m_inode->format(env);
            env.m_fscore->blockformat(addr);
            return true;
        }//if
        return false;
    };//lambda check_then_delete_node(addr)

    /**
     * @brief fill in the blank after the `nodename` is deleted
     */
    auto addr_fill_in = [&](INode & inode, int level1, int level2) -> void {
        if(inode.m_blocks_num == 0)
            return;

        int16_t filler = -1; // filler is the block address to fillin the blank at level1, level2
        
        std::size_t index = 0;
        // unary function is to get and remove the last block address from inode
        auto unary = [&](INode & inode, int16_t & block, bool) -> bool {
            ++index;
            if(index == (std::size_t)inode.m_blocks_num) {
                filler = block;
                block = 0;
                --inode.m_blocks_num;
                return false;
            }//if
            return true;
        };//lambda
        transform(env, inode, 0, unary);
        if(filler == -1) {
            std::cerr << "fs_delete: Corrupted INode data structure" << std::endl;
            std::abort();
        }//if

        if(level1 < 10) {
            inode.m_addr[level1] = filler;
        } else {
            auto level1data = env.m_fscore->blockread(inode.m_addr[level1], 1);
            int16_t * pint16_t = (int16_t *)level1data.data();
            pint16_t[level2] = filler;
        }//if-else
    };//lambda addr_fill_in(inode, level1, level2)

    // --------------------------------------------------
    Attribute attr;
    auto attrdat = env.m_fscore->blockread(env.m_attr_addr, 1);
    if(!bscan_str(attr, attrdat.begin(), attrdat.end())) {
        std::cerr << "fs_delete: parent directory Attribute load error!" << std::endl;
        std::abort();
    }//if

    // 1-level-index
    int16_t i = 0;
    for(i = 0; i < 10; ++i) {
        if(attr.m_inode->m_addr[i] == 0)
            return -1;
        if(check_then_format_node(attr.m_inode->m_addr[i])) {
            attr.m_inode->m_addr[i] = 0;
            --attr.m_inode->m_blocks_num;
            addr_fill_in(*attr.m_inode, i, -1);
            return 1;
        }//if
    }//for

    const std::size_t addrnums = env.m_fscore->fs_data_max_sz()/sizeof(int16_t);
    // 2-level-index
    for(i = 10; i < INode::c_addrnum; ++i) {
        auto level1 = env.m_fscore->blockread(attr.m_inode->m_addr[i], 1);
        int16_t * pint16_t = (int16_t *)level1.data();
        for(std::size_t j = 0; j < addrnums; ++j) {
            if(pint16_t[j] == 0)
                return -1;
            if(check_then_format_node(pint16_t[j])) {
                pint16_t[j] = 0;
                --attr.m_inode->m_blocks_num;
                addr_fill_in(*attr.m_inode, i, j);
                return 1;
            }//if
        }//for
    }//for

    return -1;
}//fs_delete(env, nodename)

/**
 * @brief works like `cd` in shell, accept `nodename` as absolute path or relative path
 */
bool
fs_change_directory(Env & env, const std::string & nodename) {
    if(nodename[0] == '/') {
        // preserve for 'commit or roll back'
        auto attr_addr = env.m_attr_addr;
        auto cur_path = env.m_cur_path;

        env.m_attr_addr = 1;
        env.m_cur_path = "/";

        boost::char_separator<char> sep("/");
        boost::tokenizer<boost::char_separator<char>> toker(nodename, sep);
        for(auto tok_iter = toker.begin(), tok_end = toker.end();
                tok_iter != tok_end; ++tok_iter)
        {
            if(!fs_change_directory(env, *tok_iter)) {
                env.m_attr_addr = attr_addr;
                env.m_cur_path = cur_path;
                return false;
            }//if
        }//for
        return true;
    } else {
        auto nodes = fs_list_directory(env);
        for(auto && i : nodes) {
            if(i.second->m_node_name == nodename) {
                env.m_cur_path += nodename;
                env.m_attr_addr = i.first;
                return true;
            }//if
        }//for
    }//if-else

    return false;
}//fs_change_directory(env, nodename)

/**
 * @brief list current directory in a vector
 *
 * note: a better performance implementation should use a class with a iterator
 */
std::vector<std::pair<int16_t ,std::shared_ptr<Attribute>>>
fs_list_directory(const Env & env) {
    std::vector<std::pair<int16_t, std::shared_ptr<Attribute>>> ret;
    auto unary = [&](INode &, int16_t & block, bool) -> bool {
        auto attr = std::make_shared<Attribute>();
        auto dat = env.m_fscore->blockread(block, 1);
        if(bscan_str(*attr, dat.begin(), dat.end())) {
            ret.emplace_back(block, attr);
        } else {
            std::cerr << "read Attribute block error in `fs_list_directory`" << std::endl;
            std::abort();
        }//if-else

        return true;
    };//lambda

    Attribute attr;
    auto attrdat = env.m_fscore->blockread(env.m_attr_addr, 1);
    if(!bscan_str(attr, attrdat.begin(), attrdat.end())) {
        std::cerr << "fs_list_directory: parent directory Attribute load error!" << std::endl;
        std::abort();
    }//if
    transform(env, *attr.m_inode, 0, unary);
    return ret;
}//fs_list_directory(env)

/**
 * @brief fs_read reads the `filename` for `size` bytes and return a char vector
 * @return on success return 1 and the corresponding char vector,
 *      failure on `filename` does not exist, return -1
 *      failure on `filename` is a directory, return -2
 *      failure on `filename` is non-readable, return -3
 */
std::pair<int, std::vector<char>>
fs_read(const Env & env, const std::string & filename, uint64_t size) {
    std::pair<int, std::vector<char>> ret;

    std::shared_ptr<Attribute> fileattr;
    auto nodelist = fs_list_directory(env);
    for(auto && i : nodelist) {
        if(i.second->m_node_name == filename) {
            if(isDir(i.second->m_flag))
                return std::make_pair(-2, std::vector<char>());
            if(!isReadable(i.second->m_flag))
                return std::make_pair(-3, std::vector<char>());
            fileattr = i.second;
            break;
        }//if
    }//for

    if(fileattr == nullptr)
        return std::make_pair(-1, std::vector<char>());

    {
        auto unary = [&](INode &, int16_t & block, bool) -> bool {
            auto blockdat = env.m_fscore->blockread(block, 1);
            ret.second.insert(ret.second.end(), blockdat.begin(), blockdat.end());
            if(ret.second.size() < size)
                return true;
            return false;
        };//lambda unary
        transform(env, *fileattr->m_inode, 0, unary);
    }//plain block

    ret.first = 1;
    return ret;
}//fs_read(env, filename, size)

/**
 * @brief the same interface as other fs_read function(s), specially, read in whole file.
 */
std::pair<int, std::vector<char>>
fs_read(const Env & env, const std::string & filename) {
    std::pair<int, std::vector<char>> ret;

    std::shared_ptr<Attribute> fileattr;
    auto nodelist = fs_list_directory(env);
    for(auto && i : nodelist) {
        if(i.second->m_node_name == filename) {
            if(isDir(i.second->m_flag))
                return std::make_pair(-2, std::vector<char>());
            if(!isReadable(i.second->m_flag))
                return std::make_pair(-3, std::vector<char>());
            fileattr = i.second;
            break;
        }//if
    }//for

    if(fileattr == nullptr)
        return std::make_pair(-1, std::vector<char>());

    {
        auto unary = [&](INode &, int16_t & block, bool) -> bool {
            auto blockdat = env.m_fscore->blockread(block, 1);
            ret.second.insert(ret.second.end(), blockdat.begin(), blockdat.end());
            return true;
        };//lambda unary
        transform(env, *fileattr->m_inode, 0, unary);
    }//plain block

    ret.first = 1;
    return ret;
}//fs_read(env, filename)

/**
 * @brief fs_write writes `data` to `filename`. `filename` has to exist in current directory.
 * @return on success, return 1;
 *      failure on filename does not exist, return -1;
 *      failure on filename being a directory, return -2;
 *      failure on filename being non-writable, return -3;
 *      failure on no more space, return -4;
 *      failure on file too big, return -5;
 */
int
fs_write(const Env & env, const std::string & filename, const std::vector<char> & data) {
    int16_t fileattr_addr;
    std::shared_ptr<Attribute> fileattr;
    auto nodelist = fs_list_directory(env);
    for(auto && i : nodelist) {
        if(i.second->m_node_name == filename) {
            if(isDir(i.second->m_flag))
                return -2;
            if(!isWritable(i.second->m_flag))
                return -3;
            fileattr_addr = i.first;
            fileattr = i.second;
            break;
        }//if
    }//for
    
    if(fileattr == nullptr)
        return -1;

    // format the file
    fileattr->m_inode->format(env);

    // if write fails, we must cleanup all stuffs that we wrote
    auto rollback = [&](int16_t block) {
        env.m_fscore->blockformat(block);
        fileattr->m_inode->format(env);
    };//lambda rollback

    auto maxdatsz = env.m_fscore->fs_data_max_sz();
    auto iter = data.begin();
    auto iter_tmp = data.begin();
    auto dist = std::distance(iter, data.end());
    while(dist > 0) {
        auto block = alloc_one_block(env);
        if(block == -1) {
            rollback(block);
            return -4;
        }//if

        if(dist > maxdatsz) {
            std::advance(iter_tmp, maxdatsz);
            dist -= maxdatsz;
        } else {
            std::advance(iter_tmp, dist);
            dist = 0;
        }//if-else

        auto blockdat = std::vector<char>(iter, iter_tmp);
        env.m_fscore->blockwrite(block, blockdat);
        iter = iter_tmp;
        
        if(!fileattr->m_inode->add_block(env, block)) {
            rollback(block);
            return -5;
        }//if
    }//while
    
    auto fileattrdat = bprint_str(*fileattr);
    env.m_fscore->blockwrite(fileattr_addr, fileattrdat);

    return 1;
}//fs_write(env, filename, data)

bool
fs_init(const std::string & fs_filename, int32_t fs_size, int16_t fs_block_size) {
    if(fs_size % fs_block_size > 0) {
        std::cerr << "fs_size must be a multiple of fs_block_size" << std::endl;
        std::abort();
    }//if

    std::ofstream ofs(fs_filename, std::ofstream::binary);
    if(!ofs.is_open()) {
        std::cerr << "fs_init error: file not found: " << fs_filename << std::endl;
        return false;
    }//if

    FSInfo info;
    {
        ofs.seekp(0, std::ios_base::beg);
        info.m_fs_size = fs_size;
        info.m_fs_block_sz = fs_block_size;

        // write FSInfo at the beginning as a block
        // * BlockInfo (16 bits, 2 bytes)
        //  * bit16: the number of bytes used
        uint16_t info_sz = FSInfo::c_struct_sz;
        ofs.write((char *)&info_sz, sizeof(uint16_t));

        auto info_binary = bprint_str(info);
        ofs.write(info_binary.data(), info_binary.size());
        ofs.flush();
    }//plain block

    // write '/' root dir attributes
    Attribute rootattr;
    {
        ofs.seekp(1 * fs_block_size, std::ios_base::beg);
        rootattr.m_flag = AttrFlag(AttrFlag::afs_dir | AttrFlag::afs_read | AttrFlag::afs_write);
        rootattr.m_owner_uid = int8_t(0); // root's uid is 0
        rootattr.m_inode = std::make_shared<INode>();
        rootattr.m_inode->m_blocks_num = 0;
        rootattr.m_node_name = "/";

        // * BlockInfo (16 bits, 2 bytes)
        //  * bit16: the number of bytes used
        uint16_t rootattr_sz = Attribute::c_struct_sz;
        ofs.write((char *)&rootattr_sz, sizeof(uint16_t));

        auto rootattr_binary = bprint_str(rootattr);
        ofs.write(rootattr_binary.data(), rootattr_binary.size());
        ofs.flush();
    }//plain block

    // reserve the disk space for the filesystem
    {
        ofs.seekp(fs_size, std::ios_base::beg);
        int32_t dummy = 0;
        ofs.write((char *)&dummy, sizeof(int32_t));
        ofs.flush();
    }//plain block

    // make a fake env
    Env fake_env;
    {
        fake_env.m_cur_uid = 0; // root's uid is 0
        fake_env.m_fscore = std::make_shared<FSCore>(fs_filename);
        fake_env.m_cur_path = "/";
        fake_env.m_attr_addr = 1;
    }//plain block

    // create "/etc/", "/home/", "/tmp/" directories
    {
        AttrFlag flag = (AttrFlag)(AttrFlag::afs_dir | AttrFlag::afs_read | AttrFlag::afs_write);
        if(!fs_create(fake_env, "etc", fake_env.m_cur_uid, flag)) {
            std::cerr << "create /etc/ failed. Inadequate space." << std::endl;
        }//if
        if(!fs_create(fake_env, "home", fake_env.m_cur_uid, flag)) {
            std::cerr << "create /home/ failed. Inadequate space." << std::endl;
        }//if
        if(!fs_create(fake_env, "tmp", fake_env.m_cur_uid, flag)) {
            std::cerr << "create /tmp/ failed. Inadequate space." << std::endl;
        }//if
    }//plain block

    // create "/etc/passwd" file
    {
        if(!fs_change_directory(fake_env, "etc")) {
            std::cerr << "unexpected cd into /etc/ failed" << std::endl;
        }//if
        AttrFlag flag = (AttrFlag)(AttrFlag::afs_read | AttrFlag::afs_write);
        if(!fs_create(fake_env, "passwd", fake_env.m_cur_uid, flag)) {
            std::cerr << "create /etc/passwd failed. Inadequate space." << std::endl;
        }//if

        const char passwdstr[] =
            "root:0\n"
            "guest:1\n";
        std::vector<char> passwddat(passwdstr, array_end(passwdstr));
        if(fs_write(fake_env, "passwd", passwddat) != 1) {
            std::cerr << "error occurs to writing to /etc/passwd" << std::endl;
        }//if
    }//plain block

    ofs.close();

    return true;
}//fs_init(fs_filename, fs_size, fs_block_sz)

}//namespace afs

