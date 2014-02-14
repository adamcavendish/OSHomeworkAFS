#pragma once

#include <cstdlib>

#ifndef NDEBUG
  #define AFS_PUBLIC public
  #define AFS_PROTECTED public
  #define AFS_PRIVATE public
#else
  #define AFS_PUBLIC public
  #define AFS_PROTECTED protected
  #define AFS_PRIVATE private
#endif//!NDEBUG

