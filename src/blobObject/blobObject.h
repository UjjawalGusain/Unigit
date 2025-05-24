#ifndef BLOBOBJECT_H
#define BLOBOBJECT_H

#include "../fileObject/fileObject.h"

#include <filesystem>
#include <string>
namespace fs = std::filesystem;

class BlobObject : public FileObject {
public:
    fs::path filePath;
    BlobObject(const fs::path &root, const std::string &source, int compressionLevel = 6);
    std::string getType() const override { return "blob"; }
    std::string getRawContent();
};

#endif
