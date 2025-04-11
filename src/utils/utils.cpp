#include <iostream>
#include <filesystem>
#include "header.h"
#include "compress.h"
namespace fs = std::filesystem;
int CHUNK_SIZE = 4096, LEVEL = 6;

std::string getObjectType(fs::path &objectPath, const std::string &hash) {
    Compressor compressor(CHUNK_SIZE, LEVEL);

    std::string header = HeaderBuilder::getHeaderFromCompressedObject(objectPath);
    try {
        if (header.rfind("blob ", 0) == 0) {
            return "blob";
        } else if(header.rfind("tree", 0) == 0){
            return "tree";
        } else if(header.rfind("commit", 0) == 0) {
            return "commit";
        } else {
            std::cerr << "Unknown object type occured" << std::endl;
            return "error";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading header: " << e.what() << '\n';
    }
    return "error";
}