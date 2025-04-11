#ifndef BLOBOBJECT_H
#define BLOBOBJECT_H

#include "fileObject.h"

#include <string>
#include <filesystem>
namespace fs = std::filesystem;

class BlobObject : public FileObject {
public:
    BlobObject(const fs::path& root, const std::string& source, int compressionLevel = 6);

    // The BlobObject reuses the FileObject implementations of write, getHash, and decompress.
    // It only needs to specify its type.
    std::string getType() const override { return "blob"; }
};

#endif  
