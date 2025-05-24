#include "../addObject/addCommand.hpp"
#include "../commitObject/commitObject.h"
#include "../utils/utils.h"
#include "json.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tchar.h>
#include <unordered_set>
#include <vector>
namespace fs = std::filesystem;
class CheckoutCommand {
public:
    static void restoreCommit(const std::string &commitHash, const fs::path &projectRootFolder) {
        std::string commitContent = readObject(projectRootFolder, commitHash);
        if (commitContent.substr(0, 6) != "commit") {
            std::cerr << "Not a valid commit object.\n";
            return;
        }

        size_t treeLineStart = commitContent.find("tree ");
        if (treeLineStart == std::string::npos) {
            std::cerr << "No tree entry in commit.\n";
            return;
        }

        size_t treeHashStart = treeLineStart + 5;
        size_t treeHashEnd = commitContent.find('\n', treeHashStart);
        std::string treeHash = commitContent.substr(treeHashStart, treeHashEnd - treeHashStart);

        restoreTree(projectRootFolder, treeHash, projectRootFolder);
    }

private:
    static std::string readObject(const fs::path &projectRootFolder, const std::string &hash) {
        fs::path objectPath = projectRootFolder / ".unigit" / "object" / hash.substr(0, 2) / hash.substr(2);
        std::ifstream file(objectPath, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open object file" << std::endl;
            return "";
        }

        std::stringstream buffer;
        std::ostream &output = buffer;

        Compressor compressor(4096, 7);
        // int result = compressor.inf(file, reinterpret_cast<std::ofstream &>(output));
        int result = compressor.inf(file, output);
        if (result != Z_OK) {
            std::cerr << "Failed to decompress object (error code: " << result << ")" << std::endl;
            return "";
        }

        return buffer.str();
    }

    static void restoreTree(const fs::path &projectRootFolder, const std::string &treeHash, const fs::path &destinationPath) {
        std::string treeContent = readObject(projectRootFolder, treeHash);
        size_t nullPos = treeContent.find('\0');
        if (nullPos == std::string::npos)
            return;

        std::string entries = treeContent.substr(nullPos + 1);
        std::istringstream iss(entries);
        std::string line;

        while (std::getline(iss, line)) {
            if (line.empty())
                continue;
            size_t spacePos = line.find(' ');
            size_t colonPos = line.find(':');

            std::string mode = line.substr(0, spacePos);
            std::string name = line.substr(spacePos + 1, colonPos - spacePos - 1);
            std::string hash = line.substr(colonPos + 1);

            fs::path targetPath = destinationPath / name;
            if (mode == "040000") {
                fs::create_directories(targetPath);
                restoreTree(projectRootFolder, hash, targetPath);
            } else {
                std::string fileContent = readObject(projectRootFolder, hash);
                size_t nullFile = fileContent.find('\0');
                if (nullFile != std::string::npos) {
                    std::ofstream outFile(targetPath, std::ios::binary);
                    outFile << fileContent.substr(nullFile + 1);
                }
            }
        }
    }
};
