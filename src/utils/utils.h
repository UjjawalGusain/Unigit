#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;
std::string getObjectType(fs::path &objectPath, const std::string &hash);
std::string readDecompressedObject(const fs::path &objectPath);
std::string sha1FromFile(const fs::path &filePath);
fs::path findUnigitRoot();
void updateHEAD(const std::string& newCommitHash, fs::path projectRootfolder);
std::string getCurrentCommitHash(fs::path projectRootfolder);
fs::path findProjectRoot();
#endif