#include <iostream>
#include <filesystem>
#include "src/blobObject/blobObject.h"

namespace fs = std::filesystem;

int main() {
    fs::path cwd = fs::current_path();
    std::string filePath = "hi.txt";

    // Step 1: Compress the file
    BlobObject blob(cwd, filePath);
    blob.write();

    std::string hash = blob.getHash();
    std::cout << "Blob object created with hash: " << hash << "\n";

    // Step 2: Prepare res folder
    fs::path resDir = cwd / "res";
    if (!fs::exists(resDir)) {
        fs::create_directory(resDir);
        std::cout << "Created 'res' directory.\n";
    }

    // Step 3: Decompress the blob to res/hi2.txt
    fs::path decompressedPath = resDir / "hi2.txt";
    blob.decompress(hash, decompressedPath);
    
    std::cout << "Decompressed file written to: " << decompressedPath << "\n";

    return 0;
}
