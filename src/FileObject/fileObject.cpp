#include "fileObject.h"
#include "../compression/compress.h"
#include "../hashing/hasher.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "../config/config.h"

namespace fs = std::filesystem;


FileObject::FileObject(const fs::path &root, const std::string &source, int compressionLevel)
    : cwd(root), sourcePath(source), level(compressionLevel) {
    tempOutputPath = "temp_object_output.blob";
}


FileObject::FileObject(const fs::path &root, const std::string &content, const std::string &objectType, int compressionLevel)
    : cwd(root), level(compressionLevel), type(objectType) {
    inputStream = std::make_unique<std::istringstream>(content, std::ios::binary);
}


void FileObject::write() {
    compressAndHash();
    moveToObjectStore();
}

std::string FileObject::getHash() const {
    return hash;
}

void FileObject::compressAndHash() {
    if (!inputStream || inputStream->fail()) {
        throw std::runtime_error("Invalid input stream");
    }

    std::ostringstream outputStream(std::ios::binary);

    Compressor compressor(CHUNK_SIZE, LEVEL);
    compressor.beginDef(*inputStream, outputStream);

    Hasher hasher;
    hasher.begin();

    inputStream->clear(); 
    inputStream->seekg(0, std::ios::beg); 

    std::vector<char> buffer(CHUNK_SIZE);
    while (!inputStream->eof()) {
        inputStream->read(buffer.data(), buffer.size());
        std::streamsize bytesRead = inputStream->gcount();

        if (bytesRead > 0) {
            compressor.addChunkDef(buffer.data(), bytesRead, inputStream->eof());
            hasher.addChunk(reinterpret_cast<const uint8_t *>(buffer.data()), bytesRead);
        }
    }

    compressor.finishDef();
    hash = hasher.finish();

    compressedData = outputStream.str();
}



void FileObject::decompress(const std::string &hash, const fs::path &outputPath) {
    fs::path objectPath = cwd / ".unigit" / "object" / hash.substr(0, 2) / hash.substr(2);

    std::ifstream input(objectPath, std::ios::binary);
    if (!input) {
        std::cerr << "Cannot open object file.\n";
        return;
    }

    std::ostringstream decompressedData;

    Compressor compressor(CHUNK_SIZE, LEVEL);
    int ret = compressor.beginInf(input, decompressedData);
    if (ret != 0) {
        std::cerr << "Failed to initialize decompression.\n";
        return;
    }

    std::vector<char> buffer(CHUNK_SIZE);
    while (true) {
        input.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = input.gcount();
        if (bytesRead <= 0) break;

        compressor.addChunkInf(buffer.data(), static_cast<size_t>(bytesRead));
    }

    compressor.finishInf();

    std::string fullData = decompressedData.str();

    auto nullPos = fullData.find('\0');
    if (nullPos == std::string::npos) {
        std::cerr << "Invalid object header.\n";
        return;
    }

    std::string content = fullData.substr(nullPos + 1);

    std::ofstream output(outputPath, std::ios::binary);
    if (!output) {
        std::cerr << "Cannot open output file.\n";
        return;
    }
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

    if (hash.size() < 3) {
        std::cerr << "Invalid hash: " << hash << "\n";
        return;
    }

    std::string firstTwoChar = hash.substr(0, 2);
    std::string objectName = hash.substr(2);
    fs::path newObjectDir = objectDir / firstTwoChar;

    if (!fs::exists(newObjectDir)) {
        fs::create_directory(newObjectDir);
    }

    fs::path finalObjectPath = newObjectDir / objectName;

    try {
        std::ofstream outputFile(finalObjectPath, std::ios::binary);
        if (!outputFile) {
            std::cerr << "Error opening file for writing: " << finalObjectPath << "\n";
            return;
        }
        outputFile.write(compressedData.data(), compressedData.size());
        outputFile.close();
    } catch (const std::exception &e) {
        std::cerr << "Error writing compressed data to object store: " << e.what() << "\n";
    }

}

