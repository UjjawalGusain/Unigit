#include "addCommand.hpp"
#include "../utils/utils.h"  // for getObjectType, if needed
#include "json.hpp"
#include "../blobObject/blobObject.h"
#define LEVEL 7


std::string AddCommand::hashAndCompressFile(fs::path entry) {
    std::cout << "Reached hashAndCompressFile function" << std::endl;

    std::string hash;
    fs::path projectRoot = findProjectRoot();

    BlobObject blob(projectRoot, entry.string(), LEVEL);
    blob.write();
    hash = blob.getHash();

    std::cout << "Reached out of hashAndCompressFile function" << std::endl;
    return hash;
}


AddCommand::AddCommand(const fs::path& root, int compressionLevel)
    : projectRoot(root), compressionLevel(compressionLevel) {}


void AddCommand::addBlobsRecursively(const fs::path& dir, const fs::path& projectRoot, nlohmann::json& watcher) {
    for (const auto& entry : fs::directory_iterator(dir)) {
        fs::path relPath = fs::relative(entry.path(), projectRoot);

        if (!relPath.empty() && relPath.begin()->string() == ".unigit")
            continue;

        if (fs::is_regular_file(entry)) {
            std::string fileHash = hashAndCompressFile(entry.path());
            watcher["added"][relPath.generic_string()] = fileHash;
            eraseIfExists(watcher["modified"], relPath.generic_string());
            eraseIfExists(watcher["new"], relPath.generic_string());
            eraseIfExists(watcher["removed"], relPath.generic_string());
        } else if (fs::is_directory(entry)) {
            addBlobsRecursively(entry.path(), projectRoot, watcher);
        }
    }
}


