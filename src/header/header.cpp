#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <fstream>
#include "compress.h"
#include "hasher.h"
#include "header.h"
namespace fs = std::filesystem;

// File would be like: <fileType> <fileSize> \0 <content>
std::string HeaderBuilder::buildBlobHeader(size_t fileSize, std::string fileType) {
    std::string header = fileType + " " + std::to_string(fileSize);
    header.push_back('\0');
    return header;
}

std::string HeaderBuilder::getHeaderFromCompressedObject(const std::filesystem::path& compressedFilePath) {
    std::ifstream input(compressedFilePath, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Could not open compressed file: " + compressedFilePath.string());
    }

    // Read a small chunk from the beginning (you only need the header)
    size_t HEADER_READ_LIMIT = 256;
    std::vector<char> compressedBuffer(HEADER_READ_LIMIT);
    input.read(compressedBuffer.data(), compressedBuffer.size());
    std::streamsize compressedSize = input.gcount();
    input.close();

    if (compressedSize == 0) {
        throw std::runtime_error("Compressed file is empty or unreadable");
    }

    // Prepare inflate state
    z_stream stream{};
    stream.next_in = reinterpret_cast<Bytef*>(compressedBuffer.data());
    stream.avail_in = static_cast<uInt>(compressedSize);

    std::vector<char> decompressed(HEADER_READ_LIMIT);

    stream.next_out = reinterpret_cast<Bytef*>(decompressed.data());
    stream.avail_out = static_cast<uInt>(decompressed.size());

    if (inflateInit(&stream) != Z_OK) {
        throw std::runtime_error("Failed to initialize zlib inflate");
    }

    int status = inflate(&stream, Z_NO_FLUSH);
    inflateEnd(&stream);

    if (status != Z_OK && status != Z_STREAM_END) {
        throw std::runtime_error("Failed to decompress header");
    }

    // Find the null-terminator
    size_t nullPos = std::find(decompressed.begin(), decompressed.end(), '\0') - decompressed.begin();
    if (nullPos == decompressed.size()) {
        throw std::runtime_error("Header not null-terminated");
    }

    return std::string(decompressed.data(), nullPos);
}