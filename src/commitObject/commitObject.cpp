#include "commitObject.h"
#include "../addObject/addCommand.hpp"
#include "../blobObject/blobObject.h"
#include "../compression/compress.h"
#include "../fileObject/fileObject.h"
#include "../hashing/hasher.h"
#include "../utils/utils.h"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
namespace fs = std::filesystem;

std::string CommitObject::writeObjectToStore(fs::path projectRoot, std::string &content) {
    std::string hash;

    try {
        FileObject fileObj(projectRoot, content, "commit");
        fileObj.write();

        hash = fileObj.getHash();
    } catch (const std::exception &e) {
        std::cerr << "Failed to write commit object: " << e.what() << "\n";
    }

    return hash;
}

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return buffer;
}

std::string CommitObject::commit(
    std::string parentHash,
    nlohmann::json &watcher,
    fs::path projectRootFolder,
    fs::path currentPath,
    const std::string author,
    const std::string description) {
    if (fs::is_regular_file(currentPath)) {
        std::string relPath = fs::relative(currentPath, projectRootFolder).generic_string();

        if (watcher["index"].contains(relPath)) {
            return watcher["index"][relPath].get<std::string>();
        } else {
            return "";
        }

    } else if (fs::is_directory(currentPath)) {
        std::vector<std::tuple<std::string, std::string, std::string>> entries;

        for (const auto &entry : fs::directory_iterator(currentPath)) {
            fs::path relPath = fs::relative(entry.path(), projectRootFolder);
            if (relPath.empty())
                continue;
            if (relPath.string().compare(0, 7, ".unigit") == 0)
                continue;

            std::string relEntryPath = relPath.generic_string();

            if (!fs::is_directory(entry) && !watcher["index"].contains(relEntryPath)) {
                continue;
            }

            std::string childHash = commit(parentHash, watcher, projectRootFolder, entry.path(), author, description);
            if (childHash.empty())
                continue;

            auto perms = fs::status(entry).permissions();
            std::string mode;
            if (fs::is_directory(entry)) {
                mode = "040000";
            } else if ((perms & fs::perms::owner_exec) != fs::perms::none) {
                mode = "100755";
            } else {
                mode = "100644";
            }

            entries.emplace_back(entry.path().filename().string(), childHash, mode);
        }

        if (entries.empty()) {
            return "";
        }

        std::sort(entries.begin(), entries.end());

        std::string combined;
        for (const auto &[name, hash, mode] : entries) {
            combined += mode + " " + name + ":" + hash + "\n";
        }

        std::string treeContent = "tree " + std::to_string(combined.size()) + '\0' + combined;

        std::string treeHash = writeObjectToStore(projectRootFolder, treeContent);

        watcher["tree"][fs::relative(currentPath, projectRootFolder).generic_string()] = treeHash;

        if (currentPath == projectRootFolder) {
            std::string payload;
            payload += "tree " + treeHash + "\n";
            if (!parentHash.empty())
                payload += "parent " + parentHash + "\n";
            payload += "author " + author + "\n";
            payload += "description " + description + "\n";
            payload += "timestamp " + getCurrentTimestamp() + "\n";

            std::string commitContent = "commit " + std::to_string(payload.size()) + '\0' + payload;

            std::string commitHash = writeObjectToStore(projectRootFolder, commitContent);
            return commitHash;
        }

        return treeHash;
    }

    return "";
}
