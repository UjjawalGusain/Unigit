#ifndef TREEOBJECT_H
#define TREEOBJECT_H
#include <iostream>
#include <filesystem>
#include <unordered_set>
namespace fs = std::filesystem;
class TreeObject {

    public:
        static std::string recursiveTraverse(std::unordered_set<fs::path> &filesToCommit, fs::path &projectRootfolder, std::string &hash, fs::path &currentFilePath);
    };

#endif