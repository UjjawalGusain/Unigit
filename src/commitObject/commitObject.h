#ifndef COMMITOBJECT_H
#define COMMITOBJECT_H
#include "../fileObject/fileObject.h"
#include "json.hpp"
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

class CommitObject : public FileObject {
public:
    CommitObject(const fs::path &root, const std::string &content)
        : FileObject(root, content, "commit") {}

    std::string getType() const override {
        return "commit";
    }

    static std::string commit(std::string parentHash, nlohmann::json &watcher, fs::path projectRootFolder, fs::path currentPath, const std::string author, const std::string description);
    static std::string writeObjectToStore(fs::path projectRoot, std::string &content);
};

#endif
