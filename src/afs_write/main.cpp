#include "afs_write/afs_write.hpp"
// STL
#include <iostream>
#include <iomanip>
#include <string>
// Boost
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
// afs
#include "afs/all.hpp"

int main(int argc, char * argv[])
{
    if(argc != 4) {
        printHelp();
        return EXIT_FAILURE;
    }//if

    auto all_env = afs::init_env(argv[1], argv[2]);

    std::string path(argv[3]);
    boost::trim(path);
    if(path.back() == '/')
        path.pop_back();

    // split the filename and the parent directory path
    auto split_filename_pos = path.find_last_of('/');
    std::string parent_path(path.begin(), path.begin() + split_filename_pos);
    std::string filename(path.begin() + split_filename_pos + 1, path.end());

    // cd into directory before read the file
    if(!afs::fs_change_directory(all_env.first, parent_path)) {
        std::cerr << "afs_write failed, path not valid." << std::endl;
        return EXIT_FAILURE;
    }//if

    std::vector<char> buffer;
    std::cin >> std::noskipws;
    while(!std::cin.eof()) {
        buffer.insert(
                buffer.end(),
                std::istream_iterator<char>(std::cin),
                std::istream_iterator<char>());
    }//while
    std::cin >> std::resetiosflags(std::ios_base::skipws);
        
    auto writecode = afs::fs_write(all_env.first, filename, buffer);
    switch(writecode) {
    case -1:
        std::cerr << "afs_write failed, file does not exist" << std::endl;
        return EXIT_FAILURE;
    case -2:
        std::cerr << "afs_write failed, cannot use afs_write to write to a directory"
            << std::endl;
        return EXIT_FAILURE;
    case -3:
        std::cerr << "afs_write failed, file is not granted write permission" << std::endl;
        return EXIT_FAILURE;
    case -4:
        std::cerr << "afs_write failed, filesystem is no more space" << std::endl;
        return EXIT_FAILURE;
    case -5:
        std::cerr << "afs_write failed, file too big to be saved in one file" << std::endl;
        return EXIT_FAILURE;
    default:
        break;
    }//switch-case

    return 0;
}//main

void
printHelp() {
    std::cerr << "usage: afs_write [afs_fsfile] [user] [file_name]" << std::endl;
    std::cerr << "example: afs_write ./myfs.fs root /tmp/testfile" << std::endl;
    std::cerr << "note: afs_write reads from stdin until EOF" << std::endl;
}//printHelp()

