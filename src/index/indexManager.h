#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>

namespace fs = std::filesystem;

struct IndexEntry {
    std::string mode;
    std::string hash;
    std::string type; // "blob" or "tree"
};

class IndexManager {
public:
    IndexManager(const fs::path& repoRoot);

    void loadIndex();
    void saveIndex();

    void addFile(const fs::path& filePath, const std::string& hash, const std::string& mode, const std::string& type);
    void removeFile(const fs::path& filePath);

    bool isStaged(const fs::path& filePath) const;
    IndexEntry getEntry(const fs::path& filePath) const;

private:
    fs::path indexPath;
    std::unordered_map<std::string, IndexEntry> indexMap;
};
