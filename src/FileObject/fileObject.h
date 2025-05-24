#ifndef FILEOBJECT_H
#define FILEOBJECT_H

#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

class FileObject {
public:
    FileObject() = default;
    FileObject(const fs::path &root, const std::string &source, int compressionLevel = 6);
    FileObject(const fs::path &root, const std::string &content, const std::string &objectType, int compressionLevel = 6);

    virtual void write();
    virtual std::string getHash() const;
    virtual void decompress(const std::string &hash, const fs::path &outputPath);

    virtual std::string getType() const {
        return type;
    }

    virtual ~FileObject() = default;

protected:
    fs::path cwd;
    std::string sourcePath;
    std::string tempOutputPath;
    std::string tempInputPath;
    int level;
    std::string hash;
    std::string type;

    void compressAndHash();
    void moveToObjectStore();
};

#endif
