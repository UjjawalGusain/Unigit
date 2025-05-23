#include "blobObject.h"
#include <fstream>
#include "../fileObject/fileObject.h"

BlobObject::BlobObject(const fs::path& root, const std::string& source, int compressionLevel)
    : FileObject(root, source, compressionLevel) {
        filePath = fs::path(source);
}

std::string BlobObject::getRawContent() {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filePath.string());
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}
