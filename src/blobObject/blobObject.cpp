#include "blobObject.h"

BlobObject::BlobObject(const fs::path& root, const std::string& source, int compressionLevel)
    : FileObject(root, source, compressionLevel) {
}
