#ifndef FILEOBJECT_H
#define FILEOBJECT_H

#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class FileObject {
public:
    FileObject() = default;
    FileObject(const fs::path& root, const std::string& source, int compressionLevel = 6);

    // Implements generic write, getHash, and decompress behavior.
    virtual void write();
    virtual std::string getHash() const;
    virtual void decompress(const std::string& hash, const fs::path& outputPath);

    // Pure virtual method forcing subclass to supply the object type (e.g., "blob", "tree")
    virtual std::string getType() const = 0; 

    virtual ~FileObject() = default;

protected:
    // Common members accessible to subclasses
    fs::path cwd;
    std::string sourcePath;
    std::string tempOutputPath;
    int level;
    std::string hash;

    // Common functions for compression, hashing and moving the object to the store.
    void compressAndHash();
    void moveToObjectStore();
};

#endif  // FILEOBJECT_H
