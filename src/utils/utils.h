#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;
std::string getObjectType(fs::path &objectPath, const std::string &hash);
#endif