#ifndef TREEOBJECT_H
#define TREEOBJECT_H

#include <string>
#include <filesystem>
#include <unordered_set>

namespace fs = std::filesystem;

class TreeObject {
public:
    static std::string recursiveTraverse(
        const std::unordered_set<fs::path> &filesToCommit,
        const fs::path &projectRoot,
        const std::string &oldHash,
        const fs::path &currentPath
    );
};

#endif // TREEOBJECT_H