#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

#include "src/commitObject/commitObject.h"
#include "src/blobObject/blobObject.h"
#include "src/commands/init/init.hpp"

namespace fs = std::filesystem;

int main() {
    // std::string filePath = "hi.txt";

    // // Step 1: Compress the file
    // BlobObject blob(cwd, filePath);
    // blob.write();

    // std::string hash = blob.getHash();
    // std::cout << "Blob object created with hash: " << hash << "\n";

    // // Step 2: Prepare res folder
    // fs::path resDir = cwd / "res";
    // if (!fs::exists(resDir)) {
    //     fs::create_directory(resDir);
    //     std::cout << "Created 'res' directory.\n";
    // }

    // Step 3: Decompress the blob to res/hi2.txt
    // fs::path decompressedPath = resDir / "hi2.txt";
    // blob.decompress(hash, decompressedPath);
    
    // std::cout << "Decompressed file written to: " << decompressedPath << "\n";
    const std::string branch = "main";
    initRepo();
    fs::path cwd = fs::current_path();

    CommitObject commitObj(branch, cwd);
    std::vector<std::string> inputPath = {"projectRootFolder"};
    std::string parentHash = "4576eac62972d855310e7a2a0b0f1e4a65d7063b78a4d7ed65f4627efafac9fc", author="UjjawalGusain", description="Descc";
    fs::path projectRootFolder = cwd / "projectRootFolder";
    std::cout << "CreateCommitObject - start" << std::endl;

    commitObj.createCommitObject(inputPath, parentHash, author, projectRootFolder);
    std::cout << "CreateCommitObject - end" << std::endl;

    // fs::path cwd = fs::current_path();
    // CommitObject c = CommitObject("main", cwd);
    // fs::path projectRootFolder = cwd / "projectRootFolder";
    // c.getParentCommitHash("d4e8115df1241808145414e76900a228089674cf5a10e75ff93beccabe058fea", projectRootFolder);

    
    return 0;
}
