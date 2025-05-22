#include "treeObject.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "../compression/compress.h"
#include "../hashing/hasher.h"
#include "../utils/utils.h"
#include "../blobObject/blobObject.h"
#include <set>

namespace fs = std::filesystem;

std::string TreeObject::recursiveTraverse(
    const std::unordered_set<fs::path> &filesToCommit,
    const fs::path &projectRoot,
    const std::string &oldHash,
    const fs::path &currentPath
) {
    std::cout << "Files to commit: ";
    for(auto &s : filesToCommit) {
        std::cout << s << " ";
    }
    std::cout << "Files to commit ends" << std::endl;
    std::cout << "TreeObject stage - 1" << std::endl;
    const int CHUNK_SIZE = 4096;
    const int LEVEL = 6;

    std::vector<std::tuple<std::string, std::string, std::string>> entries;

    bool isFirstCommit = oldHash.empty();
    fs::path objFile;

    if (!isFirstCommit && fs::exists(objFile)) {
        objFile = projectRoot / ".unigit" / "object" / oldHash.substr(0,2) / oldHash.substr(2);
        std::cout << "TreeObject stage - 1.5" << std::endl;
        std::string data = readDecompressedObject(objFile);
        std::istringstream iss(data);
        std::cout << "TreeObject stage - 2" << std::endl;

        std::string header;
        std::getline(iss, header);
        std::cout << "TreeObject stage - 3" << std::endl;

        std::string line;
        while (std::getline(iss, line)) {
            if (line.empty()) continue;
            std::istringstream ls(line);
            std::string type, name, hash;
            ls >> type >> name >> hash;
            entries.emplace_back(type, name, hash);
        }
    } else {
        std::cout << "TreeObject: First commit case triggered.\n";
        // Only include entries from filesToCommit that are direct children of currentPath
        std::set<fs::path> uniqueChildren;

        for (const auto &f : filesToCommit) {
            if (fs::relative(f, currentPath).empty()) continue;

            fs::path relative = fs::relative(f, currentPath);

            // Check if relative path starts with ".."
            auto it = relative.begin();
            if (it != relative.end() && *it == "..") {
                // This file is outside the currentPath subtree, skip it
                continue;
            }
        
            fs::path firstComponent = *relative.begin(); // first directory or file
            uniqueChildren.insert(currentPath / firstComponent);
        }
        std::cout << "TreeObject(first commit) - 2" << std::endl;
        for (const auto &child : uniqueChildren) {
            std::string type, hash;
            std::string name = child.filename().string();
            std::cout << "Child name: " << name << std::endl;
            if (fs::is_directory(child)) {
                hash = recursiveTraverse(filesToCommit, projectRoot, "", child);
                type = "tree";
            } else {
                BlobObject blob(projectRoot, child.string(), LEVEL);
                blob.write();
                hash = blob.getHash();
                type = "blob";
            }
            entries.emplace_back(type, name, hash);
            std::cout << "TreeObject(first commit) - 3" << std::endl;

        }
    }

    std::cout << "TreeObject stage - 4" << std::endl;

    std::ostringstream newContent;
    for (auto &e : entries) {
        const auto &type = std::get<0>(e);
        const auto &name = std::get<1>(e);
        const auto &hash = std::get<2>(e);
        fs::path childPath = currentPath / name;
        std::cout << "TreeObject stage - 5: " << childPath << std::endl;

        std::string newHash;
        if (type == "blob") {
            if (filesToCommit.count(childPath)) {
                BlobObject blob(projectRoot, childPath.string(), LEVEL);
                blob.write();
                newHash = blob.getHash();
            } else {
                newHash = hash;
            }
        } else { // tree
            newHash = recursiveTraverse(filesToCommit, projectRoot, hash, childPath);
        }
        newContent << type << " " << name << " " << newHash << "\n";
    }

    std::cout << "TreeObject stage - 6" << std::endl;

    std::string body = newContent.str();
    std::string full = "tree " + std::to_string(body.size()) + '\0' + body;

    // Hash, compress, and store new tree
    Hasher h = Hasher();
    std::string newTreeHash = h.hashString(full);
    Compressor cmp(CHUNK_SIZE, LEVEL);
    std::ofstream outTmp("temp_tree_obj"); outTmp << full; outTmp.close();
    fs::path outDir = projectRoot / ".unigit" / "object" / newTreeHash.substr(0,2);
    std::cout << "TreeObject stage - 7" << std::endl;
    fs::create_directories(outDir);
    std::ofstream outObj(outDir / newTreeHash.substr(2), std::ios::binary);
    std::ifstream inTmp("temp_tree_obj");
    cmp.def(inTmp, outObj);
    inTmp.close(); outObj.close();
    fs::remove("temp_tree_obj");
    std::cout << "TreeObject stage - 8" << std::endl;

    return newTreeHash;
}
