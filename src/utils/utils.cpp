// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <filesystem>

// Decompress a stored object file and return its full contents
std::string readDecompressedObject(const std::filesystem::path &objectPath);

// Compute SHA1 (or equivalent) hash of a file using existing Hasher
std::string sha1FromFile(const std::filesystem::path &filePath);

#endif // UTILS_H

// utils.cpp
#include "utils.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "../compression/compress.h"
#include "../hashing/hasher.h"
#include <filesystem>
#include <stdexcept>
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

// Decompress a stored object file and return its full contents (including header and body)
std::string readDecompressedObject(const std::filesystem::path &objectPath) {
    std::ifstream input(objectPath, std::ios::binary);
    if (!input) {
        std::cerr << "Error opening object file for decompression: " << objectPath << "\n";
        return {};
    }

    std::ostringstream decompressed;
    Compressor compressor(4096, 6);
    compressor.beginInf(input, decompressed);

    std::vector<char> buffer(4096);
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

// Compute SHA1 hash of a file using the existing Hasher class
std::string sha1FromFile(const std::filesystem::path &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot open file for hashing: " << filePath << "\n";
        return {};
    }

    Hasher hasher;
    hasher.begin();

    std::vector<char> buffer(4096);
    while (!file.eof()) {
        file.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = file.gcount();
        if (bytesRead > 0) {
            hasher.addChunk(reinterpret_cast<const uint8_t*>(buffer.data()), static_cast<size_t>(bytesRead));
        }
    }

    return hasher.finish();
}

void updateHEAD(const std::string& newCommitHash, fs::path projectRootfolder) {
    std::ofstream headFile(projectRootfolder / ".unigit" / "HEAD" , std::ios::trunc);
    if (!headFile.is_open()) {
        throw std::runtime_error("Failed to open .unigit/HEAD for writing.");
    }

    headFile << newCommitHash;
    headFile.close();
}

std::string getCurrentCommitHash(fs::path projectRootfolder) {
    std::ifstream headFile(projectRootfolder / ".unigit" / "HEAD" , std::ios::trunc);
    if (!headFile.is_open()) {
        throw std::runtime_error("Unable to open .unigit/HEAD");
    }

    std::string hash;
    std::getline(headFile, hash);
    headFile.close();

    if (hash.empty()) {
        throw std::runtime_error("HEAD is empty");
    }

    return hash;
}