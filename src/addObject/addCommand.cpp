#include "addCommand.hpp"
#include "../blobObject/blobObject.h"
#include "../utils/utils.h"
#include "BS_thread_pool.hpp" 
#include "json.hpp"
#include "../config/config.h"
AddCommand::AddCommand(const fs::path &root, int compressionLevel) {}

std::string AddCommand::writeObjectToStore(fs::path projectRoot, std::string &content) {
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

std::string AddCommand::hashAndCompressFile(fs::path entry) {
    fs::path projectRoot = findProjectRoot();

    BlobObject blob(projectRoot, entry.string(), LEVEL);
    std::string rawContent = blob.getRawContent();
    std::string contentWithHeader = "blob " + std::to_string(rawContent.size()) + '\0' + rawContent;

    std::string hash = writeObjectToStore(projectRoot, contentWithHeader);
    return hash;
}

std::string AddCommand::hashFile(fs::path filePath) {
    Hasher hasher;
    hasher.begin();

    std::ifstream input(filePath, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Cannot open input file: " + filePath.string());
    }

    input.seekg(0, std::ios::end);
    size_t fileSize = input.tellg();
    input.seekg(0, std::ios::beg);

    std::string header = "blob " + std::to_string(fileSize) + '\0';
    hasher.addChunk(reinterpret_cast<const uint8_t *>(header.data()), header.size());

    std::vector<char> buffer(CHUNK_SIZE);
    while (!input.eof()) {
        input.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = input.gcount();
        if (bytesRead > 0) {
            hasher.addChunk(reinterpret_cast<const uint8_t *>(buffer.data()), bytesRead);
        }
    }

    std::string hash = hasher.finish();
    return hash;
}

void AddCommand::addBlobsOnce(const std::vector<fs::path>& basePaths, const fs::path &projectRoot, nlohmann::json &watcher) {
    BS::thread_pool pool(16);
    std::mutex watcherMutex;

    std::function<void(fs::path)> processEntry = [&](fs::path path) {
        fs::path relPath = fs::relative(path, projectRoot);
        if (!relPath.empty() && relPath.begin()->string() == ".unigit") return;

        if (fs::is_regular_file(path)) {
            std::string fileHash = hashFile(path);
            fs::path objectPath = projectRoot / ".unigit" / "object" / fileHash.substr(0, 2) / fileHash.substr(2);

            {
                std::lock_guard<std::mutex> lock(watcherMutex);
                bool alreadyTracked = watcher["added"].contains(relPath.generic_string()) &&
                                      watcher["added"][relPath.generic_string()] == fileHash;
                bool objectExists = fs::exists(objectPath);

                if (!objectExists) {
                    hashAndCompressFile(path);
                }

                if (!alreadyTracked) {
                    watcher["added"][relPath.generic_string()] = fileHash;
                    watcher["index"][relPath.generic_string()] = fileHash;
                    eraseIfExists(watcher["modified"], relPath.generic_string());
                    eraseIfExists(watcher["new"], relPath.generic_string());
                    eraseIfExists(watcher["removed"], relPath.generic_string());
                }

                std::cout << "File staged: " << path << std::endl;
            }
        } else if (fs::is_directory(path)) {
            for (const auto &entry : fs::directory_iterator(path)) {
                pool.detach_task([=] { processEntry(entry.path()); });
            }
        }
    };

    for (const auto &base : basePaths) {
        if (fs::exists(base)) {
            pool.detach_task([=] { processEntry(base); });
        }
    }

    pool.wait();
}