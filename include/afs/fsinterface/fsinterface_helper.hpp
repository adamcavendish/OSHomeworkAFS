#pragma once

// C-STL
#include <cstdint>
// afs
#include "afs/env/env.hpp"

namespace afs {

int16_t
alloc_one_block(const Env & env);

}//namespace afs


