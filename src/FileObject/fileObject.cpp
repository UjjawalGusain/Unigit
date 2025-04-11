#include "fileObject.h"
#include "../compression/compress.h"
#include "../hashing/hasher.h"
#include "../header/header.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;
size_t CHUNK = 4096;
int LEVEL = 6;


FileObject::FileObject(const fs::path& root, const std::string& source, int compressionLevel)
    : cwd(root), sourcePath(source), level(compressionLevel) {
    // Setting a temporary file name for object creation.
    tempOutputPath = "temp_object_output.blob";
}

void FileObject::write() {
    compressAndHash();
    moveToObjectStore();
}

std::string FileObject::getHash() const {
    return hash;
}

void FileObject::compressAndHash() {
    std::ifstream input(sourcePath, std::ios::binary);
    std::ofstream output(tempOutputPath, std::ios::binary);

    if (!input || !output) {
        std::cerr << "Error opening source or destination file.\n";
        return;
    }

    Compressor compressor(CHUNK, LEVEL);
    compressor.beginDef(input, output);

    Hasher hasher;
    hasher.begin();

    // Determine file size for header preparation.
    input.seekg(0, std::ios::end);
    size_t fileSize = input.tellg();
    input.seekg(0, std::ios::beg);

    // Use the virtual getType() to build an appropriate header.
    std::string header = HeaderBuilder::buildBlobHeader(fileSize, getType());
    compressor.addChunkDef(header.data(), header.size());
    hasher.addChunk(reinterpret_cast<const uint8_t*>(header.data()), header.size());

    std::vector<char> buffer(4096);
    while (!input.eof()) {
        input.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = input.gcount();

        if (bytesRead > 0) {
            compressor.addChunkDef(buffer.data(), bytesRead, input.eof());
            hasher.addChunk(reinterpret_cast<const uint8_t*>(buffer.data()), bytesRead);
        }
    }

    compressor.finishDef();
    hash = hasher.finish();
} 

void FileObject::decompress(const std::string& hash, const fs::path& outputPath) {
    fs::path objectPath = cwd / ".unigit" / "object" / hash.substr(0, 2) / hash.substr(2);

    std::ifstream input(objectPath, std::ios::binary);
    if (!input) {
        std::cerr << "Cannot open object file.\n";
        return;
    }

    std::ostringstream decompressedData;
    Compressor compressor(CHUNK, LEVEL);
    compressor.beginInf(input, decompressedData);

    std::vector<char> buffer(4096);
    while (!input.eof()) {
        input.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = input.gcount();
        if (bytesRead > 0) {
            compressor.addChunkInf(buffer.data(), bytesRead);
        }
    }
    compressor.finishInf();

    std::string fullData = decompressedData.str();

    // Locate the header end marker (null character)
    auto nullPos = fullData.find('\0');
    if (nullPos == std::string::npos) {
        std::cerr << "Invalid object header.\n";
        return;
    }

    // Skip the header and write only the content.
    std::string content = fullData.substr(nullPos + 1);

    std::ofstream output(outputPath, std::ios::binary);
    output.write(content.data(), content.size());
    
}

void FileObject::moveToObjectStore() {
    fs::path unigitDir = cwd / ".unigit";
    if (!fs::exists(unigitDir)) {
        std::cerr << ".unigit is not initialized\n";
        return;
    }

    fs::path objectDir = unigitDir / "object";
    if (!fs::exists(objectDir)) {
        fs::create_directory(objectDir);
    }

    std::string firstTwoChar = hash.substr(0, 2);
    std::string objectName = hash.substr(2);
    fs::path newObjectDir = objectDir / firstTwoChar;

    if (!fs::exists(newObjectDir)) {
        fs::create_directory(newObjectDir);
    }

    fs::path finalObjectPath = newObjectDir / objectName;

    try {
        fs::rename(tempOutputPath, finalObjectPath);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error moving object file: " << e.what() << "\n";
    }
}
