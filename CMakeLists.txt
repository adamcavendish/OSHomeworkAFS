CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall -march=native -flto -O3")
#ADD_DEFINITIONS(-DNDEBUG)

SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall -march=native -gdwarf-2 -g3")

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/include/")

# --------------------------------------------------
# Add gtest
# --------------------------------------------------
INCLUDE(ExternalProject)
# Set default ExternalProject root directory
set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/ThirdParty)
ExternalProject_Add(
    googletest
    SVN_REPOSITORY http://googletest.googlecode.com/svn/trunk/
    SVN_REVISION -r 660
    TIMEOUT 1
	# Disable update step
	UPDATE_COMMAND ""
    # Disable install step
    INSTALL_COMMAND ""
    # Wrap download, configure and build steps in a script to log output
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)
# Specify include dir
ExternalProject_Get_Property(googletest source_dir)
INCLUDE_DIRECTORIES("${source_dir}/include/")
ExternalProject_Get_Property(googletest binary_dir)
LINK_DIRECTORIES("${binary_dir}/")
# --------------------------------------------------
# Add gtest fin.
# --------------------------------------------------

# PROJECT
PROJECT(OSHomeworkAFS)
MESSAGE(STATUS "Project: " ${PROJECT_NAME})

SET(__TARGET_NAME__ afs_ls)
SET(__SRC__
    ./src/afs_ls/main.cpp
    ./src/ghelper.cpp
    ./src/afs/fscore/fscore.cpp
    ./src/afs/inode/inode.cpp
    ./src/afs/fsinterface/fsinterface.cpp
    ./src/afs/fsinterface/fsinterface_helper.cpp
)
ADD_EXECUTABLE(${__TARGET_NAME__} ${__SRC__})

SET(__TARGET_NAME__ afs_touch)
SET(__SRC__
    ./src/afs_touch/main.cpp
    ./src/ghelper.cpp
    ./src/afs/fscore/fscore.cpp
    ./src/afs/inode/inode.cpp
    ./src/afs/fsinterface/fsinterface.cpp
    ./src/afs/fsinterface/fsinterface_helper.cpp
)
ADD_EXECUTABLE(${__TARGET_NAME__} ${__SRC__})

SET(__TARGET_NAME__ afs_createfs)
SET(__SRC__
    ./src/afs_createfs/main.cpp
    ./src/ghelper.cpp
    ./src/afs/fscore/fscore.cpp
    ./src/afs/inode/inode.cpp
    ./src/afs/fsinterface/fsinterface.cpp
    ./src/afs/fsinterface/fsinterface_helper.cpp
)
ADD_EXECUTABLE(${__TARGET_NAME__} ${__SRC__})

SET(__TARGET_NAME__ afs_chmod)
SET(__SRC__
    ./src/afs_chmod/main.cpp
    ./src/ghelper.cpp
    ./src/afs/fscore/fscore.cpp
    ./src/afs/inode/inode.cpp
    ./src/afs/fsinterface/fsinterface.cpp
    ./src/afs/fsinterface/fsinterface_helper.cpp
)
ADD_EXECUTABLE(${__TARGET_NAME__} ${__SRC__})

SET(__TARGET_NAME__ afs_rm)
SET(__SRC__
    ./src/afs_rm/main.cpp
    ./src/ghelper.cpp
    ./src/afs/fscore/fscore.cpp
    ./src/afs/inode/inode.cpp
    ./src/afs/fsinterface/fsinterface.cpp
    ./src/afs/fsinterface/fsinterface_helper.cpp
)
ADD_EXECUTABLE(${__TARGET_NAME__} ${__SRC__})

SET(__TARGET_NAME__ afs_read)
SET(__SRC__
    ./src/afs_read/main.cpp
    ./src/ghelper.cpp
    ./src/afs/fscore/fscore.cpp
    ./src/afs/inode/inode.cpp
    ./src/afs/fsinterface/fsinterface.cpp
    ./src/afs/fsinterface/fsinterface_helper.cpp
)
ADD_EXECUTABLE(${__TARGET_NAME__} ${__SRC__})

SET(__TARGET_NAME__ afs_write)
SET(__SRC__
    ./src/afs_write/main.cpp
    ./src/ghelper.cpp
    ./src/afs/fscore/fscore.cpp
    ./src/afs/inode/inode.cpp
    ./src/afs/fsinterface/fsinterface.cpp
    ./src/afs/fsinterface/fsinterface_helper.cpp
)
ADD_EXECUTABLE(${__TARGET_NAME__} ${__SRC__})

SET(__TARGET_NAME__ afs_adduser)
SET(__SRC__
    ./src/afs_adduser/main.cpp
    ./src/ghelper.cpp
    ./src/afs/fscore/fscore.cpp
    ./src/afs/inode/inode.cpp
    ./src/afs/fsinterface/fsinterface.cpp
    ./src/afs/fsinterface/fsinterface_helper.cpp
)
ADD_EXECUTABLE(${__TARGET_NAME__} ${__SRC__})

SET(__TARGET_NAME__ afs_rmuser)
SET(__SRC__
    ./src/afs_rmuser/main.cpp
    ./src/ghelper.cpp
    ./src/afs/fscore/fscore.cpp
    ./src/afs/inode/inode.cpp
    ./src/afs/fsinterface/fsinterface.cpp
    ./src/afs/fsinterface/fsinterface_helper.cpp
)
ADD_EXECUTABLE(${__TARGET_NAME__} ${__SRC__})

# --------------------------------------------------
# TESTS
# --------------------------------------------------
SET(__TEST_NAME__ test001)
SET(__TARGET_NAME__ test001)
SET(__SRC__
    ./test/test001.cpp
    ./src/ghelper.cpp
    ./src/afs/fscore/fscore.cpp
    ./src/afs/inode/inode.cpp
    ./src/afs/fsinterface/fsinterface.cpp
    ./src/afs/fsinterface/fsinterface_helper.cpp
)
ADD_EXECUTABLE(${__TARGET_NAME__} ${__SRC__})
TARGET_LINK_LIBRARIES(${__TARGET_NAME__} gtest gtest_main pthread)
ADD_TEST(${__TEST_NAME__} ${__TARGET_NAME__})

