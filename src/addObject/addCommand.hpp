// AddCommand.h
#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include "../compression/compress.h"
#include "../hashing/hasher.h"
#include "json.hpp"

namespace fs = std::filesystem;

class AddCommand {
public:
    AddCommand(const fs::path& root, int compressionLevel = 6);
    
    static std::string hashAndCompressFile(fs::path entry);
    void addBlobsRecursively(const fs::path& dir, const fs::path& projectRoot, nlohmann::json& watcher);

private:
    fs::path projectRoot;
    int compressionLevel;

};

