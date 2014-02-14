#pragma once
// STL
#include <string>
#include <vector>
// afs
#include "afs/env/env.hpp"
#include "afs/attribute/flag.hpp"

namespace afs {

bool
fs_create(Env & env, const std::string & filename, int8_t owner_uid, const AttrFlag & flag);

bool
fs_delete(Env & env, const std::string & filename);

std::pair<bool, std::vector<char>>
fs_read(Env & env, int32_t fd, uint64_t size);

bool
fs_write(Env & env, int32_t fd, const std::vector<char> & data);

int32_t
fs_open(Env & env, const std::string & filename, const AttrFlag & flag);

bool
fs_close(int32_t fd);

bool
fs_init(const std::string & fs_filename, int32_t fs_size, int32_t fs_block_size);

}//namespace afs

