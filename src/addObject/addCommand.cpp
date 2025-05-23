#include "addCommand.hpp"
#include "../utils/utils.h"  // for getObjectType, if needed
#include "json.hpp"
#include "../blobObject/blobObject.h"
#include "../header/header.h"
#define LEVEL 7

AddCommand::AddCommand(const fs::path& root, int compressionLevel) {}

// std::string AddCommand::hashAndCompressFile(fs::path entry) {
//     std::cout << "Reached hashAndCompressFile function" << std::endl;

//     std::string hash;
//     fs::path projectRoot = findProjectRoot();

//     BlobObject blob(projectRoot, entry.string(), LEVEL);
//     blob.write();
//     hash = blob.getHash();

//     std::cout << "Reached out of hashAndCompressFile function" << std::endl;
//     return hash;
// }

std::string AddCommand::writeObjectToStore(fs::path projectRoot, std::string& content) {
    std::string hash;

    try {
        // Use the constructor that handles content directly
        FileObject fileObj(projectRoot, content, "commit");  // Sets up temp file and type
        fileObj.write();  // Compress + hash + move to object store

        hash = fileObj.getHash();
    } catch (const std::exception& e) {
        std::cerr << "Failed to write commit object: " << e.what() << "\n";
    }

    return hash;
}



std::string AddCommand::hashAndCompressFile(fs::path entry) {
    // std::cout << "Reached hashAndCompressFile function" << std::endl;

    fs::path projectRoot = findProjectRoot();

    BlobObject blob(projectRoot, entry.string(), LEVEL);
    std::string rawContent = blob.getRawContent();

    // Add header for blob object, just like for tree and commit
    std::string contentWithHeader = "blob " + std::to_string(rawContent.size()) + '\0' + rawContent;

    // Write this to object store, get hash
    std::string hash = writeObjectToStore(projectRoot, contentWithHeader);

    // std::cout << "Reached out of hashAndCompressFile function" << std::endl;
    return hash;
}


std::string AddCommand::hashFile(fs::path filePath) {
    Hasher hasher;
    hasher.begin();

    std::ifstream input(filePath, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Cannot open input file: " + filePath.string());
    }

    // Prepare header similar to compressAndHash (but no compression)
    input.seekg(0, std::ios::end);
    size_t fileSize = input.tellg();
    input.seekg(0, std::ios::beg);

    std::string header = HeaderBuilder::buildBlobHeader(fileSize, "blob"); // or getType() if needed
    hasher.addChunk(reinterpret_cast<const uint8_t*>(header.data()), header.size());

    std::vector<char> buffer(4096);
    while (!input.eof()) {
        input.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = input.gcount();
        if (bytesRead > 0) {
            hasher.addChunk(reinterpret_cast<const uint8_t*>(buffer.data()), bytesRead);
        }
    }

    std::string hash = hasher.finish();
    return hash;
}


void AddCommand::addBlobsRecursively(const fs::path& dir, const fs::path& projectRoot, nlohmann::json& watcher) {
    for (const auto& entry : fs::directory_iterator(dir)) {
        fs::path relPath = fs::relative(entry.path(), projectRoot);

        if (!relPath.empty() && relPath.begin()->string() == ".unigit")
            continue;

        if (fs::is_regular_file(entry)) {
            // Step 1: hash only, no compression yet
            std::string fileHash = hashFile(entry.path());
            std::cout << "File staged: " << entry.path() << std::endl;
            // std::cout << "fileHash: " << fileHash << std::endl;
            fs::path objectPath = projectRoot / ".unigit" / "object" / fileHash.substr(0, 2) / fileHash.substr(2);

            bool alreadyTracked = watcher["added"].contains(relPath.generic_string()) &&
                                  watcher["added"][relPath.generic_string()] == fileHash;
            bool objectExists = fs::exists(objectPath);

            // Step 2: compress and store only if needed
            if (!objectExists) {
                hashAndCompressFile(entry.path());
            }

            // Step 3: update watcher only if file contents changed
            if (!alreadyTracked) {
                watcher["added"][relPath.generic_string()] = fileHash;
                eraseIfExists(watcher["modified"], relPath.generic_string());
                eraseIfExists(watcher["new"], relPath.generic_string());
                eraseIfExists(watcher["removed"], relPath.generic_string());
            }

        } else if (fs::is_directory(entry)) {
            addBlobsRecursively(entry.path(), projectRoot, watcher);
        }
    }
}




