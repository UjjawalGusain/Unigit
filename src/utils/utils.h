#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <filesystem>
#include "json.hpp"
namespace fs = std::filesystem;
std::string readDecompressedObject(const fs::path &objectPath);
std::string sha1FromFile(const fs::path &filePath);
fs::path findUnigitRoot();
void updateHEAD(const std::string& newCommitHash, fs::path projectRootfolder);
std::string getCurrentCommitHash(fs::path projectRootfolder);
fs::path findProjectRoot();
std::string readFile(const fs::path &filePath);
std::string sha1FromString(const std::string& input);
void eraseIfExists(nlohmann::json& arr, const std::string& val);
#endif