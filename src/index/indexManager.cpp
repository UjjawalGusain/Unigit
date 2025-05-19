#include "indexManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

IndexManager::IndexManager(const fs::path& repoRoot) {
    indexPath = repoRoot / ".unigit" / "index";
}

void IndexManager::loadIndex() {
    indexMap.clear();
    std::ifstream inFile(indexPath);
    if (!inFile) return;

    std::string line;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::string mode, relPath, hash, type;
        if (iss >> mode >> relPath >> hash >> type) {
            indexMap[relPath] = {mode, hash, type};
        }
    }
}

void IndexManager::saveIndex() {
    std::ofstream outFile(indexPath, std::ios::trunc);
    for (const auto& [relPath, entry] : indexMap) {
        outFile << entry.mode << " " << relPath << " " << entry.hash << " " << entry.type << "\n";
    }
}

void IndexManager::addFile(const fs::path& filePath, const std::string& hash, const std::string& mode, const std::string& type) {
    fs::path relPath = fs::relative(filePath, indexPath.parent_path().parent_path());
    indexMap[relPath.string()] = {mode, hash, type};
}

void IndexManager::removeFile(const fs::path& filePath) {
    fs::path relPath = fs::relative(filePath, indexPath.parent_path().parent_path());
    indexMap.erase(relPath.string());
}

bool IndexManager::isStaged(const fs::path& filePath) const {
    fs::path relPath = fs::relative(filePath, indexPath.parent_path().parent_path());
    return indexMap.find(relPath.string()) != indexMap.end();
}

IndexEntry IndexManager::getEntry(const fs::path& filePath) const {
    fs::path relPath = fs::relative(filePath, indexPath.parent_path().parent_path());
    auto it = indexMap.find(relPath.string());
    return it != indexMap.end() ? it->second : IndexEntry{};
}
