#pragma once
// STL
#include <string>
#include <vector>
// afs
#include "afs/env/env.hpp"
#include "afs/attribute/attribute.hpp"
#include "afs/attribute/flag.hpp"

namespace afs {

bool
fs_create(Env & env, const std::string & nodename, int8_t owner_uid, const AttrFlag & flag);

bool
fs_delete(Env & env, const std::string & nodename);

bool
fs_change_directory(Env & env, const std::string & nodename);

std::vector<std::shared_ptr<Attribute>>
fs_list_directory(const Env & env);

std::pair<bool, std::vector<char>>
fs_read(Env & env, const std::string & filename, uint64_t size);

bool
fs_write(Env & env, const std::string & filename, const std::vector<char> & data);

bool
fs_init(const std::string & fs_filename, int32_t fs_size, int16_t fs_block_size);

}//namespace afs

