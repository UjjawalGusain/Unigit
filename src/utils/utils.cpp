#include "utils.h"
#include "../compression/compress.h"
#include "../hashing/hasher.h"
#include "json.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "../config/config.h"
namespace fs = std::filesystem;

fs::path findUnigitRoot() {
    fs::path current = fs::current_path();
    while (!current.empty()) {
        if (fs::exists(current / ".unigit") && fs::is_directory(current / ".unigit")) {
            return current;
        }
        current = current.parent_path();
    }
    throw std::runtime_error("Not a unigit repository (or any of the parent directories): .unigit");
}

void eraseIfExists(nlohmann::json &arr, const std::string &val) {
    if (!arr.is_array())
        return;

    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i] == val) {
            arr.erase(i);
            return;
        }
    }
}

std::string readDecompressedObject(const std::filesystem::path &objectPath) {
    std::ifstream input(objectPath, std::ios::binary);
    if (!input) {
        std::cerr << "Error opening object file for decompression: " << objectPath << "\n";
        return {};
    }

    std::ostringstream decompressed;
    Compressor compressor(CHUNK_SIZE, LEVEL);
    compressor.beginInf(input, decompressed);

    std::vector<char> buffer(CHUNK_SIZE);
    while (!input.eof()) {
        input.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = input.gcount();
        if (bytesRead > 0) {
            compressor.addChunkInf(buffer.data(), static_cast<size_t>(bytesRead));
        }
    }
    compressor.finishInf();
    return decompressed.str();
}

std::string sha1FromFile(const std::filesystem::path &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot open file for hashing: " << filePath << "\n";
        return {};
    }

    Hasher hasher;
    hasher.begin();

    std::vector<char> buffer(CHUNK_SIZE);
    while (!file.eof()) {
        file.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = file.gcount();
        if (bytesRead > 0) {
            hasher.addChunk(reinterpret_cast<const uint8_t *>(buffer.data()), static_cast<size_t>(bytesRead));
        }
    }

    return hasher.finish();
}

std::string sha1FromString(const std::string &input) {
    Hasher hasher;
    hasher.begin();

    size_t chunkSize = CHUNK_SIZE;
    size_t offset = 0;
    while (offset < input.size()) {
        size_t len = std::min(chunkSize, input.size() - offset);
        hasher.addChunk(reinterpret_cast<const uint8_t *>(input.data() + offset), len);
        offset += len;
    }

    return hasher.finish();
}

std::string readFile(const fs::path &filePath) {
    std::cout << "Reached readFile function" << std::endl;
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filePath.string());
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::cout << "Reached out of readFile function" << std::endl;
    return ss.str();
}

void updateHEAD(const std::string &newCommitHash, fs::path projectRootfolder) {
    fs::path headPath = projectRootfolder / ".unigit" / "HEAD";

    std::ofstream headFile(headPath, std::ios::trunc);
    if (!headFile.is_open()) {
        throw std::runtime_error("Failed to open or create .unigit/HEAD for writing.");
    }

    headFile << newCommitHash;
    headFile.close();
}

std::string getCurrentCommitHash(fs::path projectRootfolder) {
    fs::path headPath = projectRootfolder / ".unigit" / "HEAD";

    if (!fs::exists(headPath)) {
        return "";
    }

    std::ifstream headFile(headPath);
    if (!headFile.is_open()) {
        throw std::runtime_error("Unable to open .unigit/HEAD");
    }

    std::string hash;
    std::getline(headFile, hash);
    headFile.close();

    return hash;
}

fs::path findProjectRoot() {
    fs::path current = fs::current_path();
    while (!fs::exists(current / ".unigit")) {
        if (!current.has_parent_path()) {
            throw std::runtime_error("Not inside a UniGit repository");
        }
        current = current.parent_path();
    }
    return current;
}