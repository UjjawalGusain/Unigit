#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <string>
#include "../compression/compress.h"
#include "../header/header.h"
#include "../hashing/hasher.h"
#include "../utils/utils.h" // gets us getObjectType function
#include "../blobObject/blobObject.h"
#include "../treeObject/treeObject.h"
#include "../fileObject/fileObject.h"
#include "commitObject.h"
namespace fs = std::filesystem;


CommitObject::CommitObject(const fs::path& root, const std::string& source, int compressionLevel)
    : FileObject(root, source, compressionLevel) {
    // No additional initialization is required for CommitObject.
}


CommitObject::CommitObject(const std::string& branch, fs::path& cwd) {
    fs::path refCommitHashFilePath = cwd / ".unigit" / "refs" / "heads" / branch;
    std::cout << "Hello there. Welcome to commit object initialization" << std::endl;

    if (!fs::exists(refCommitHashFilePath)) {
        refCommitHash = "0000000000000000000000000000000000000000";
    } else {
        std::ifstream inputCommitFile(refCommitHashFilePath, std::ios::binary);
        if (!inputCommitFile) {
            std::cerr << "Error: Could not open file: " << refCommitHashFilePath << "\n";
            refCommitHash = "0000000000000000000000000000000000000000";
            return;
        }

        getline(inputCommitFile, refCommitHash);
        inputCommitFile.close();
    }
    std::cout << "Hello there. Welcome to commit object end. This is refCommitHash: " << refCommitHash << std::endl;
}



void CommitObject::commit(std::vector<std::string> commitPaths) {
    fs::path projectRoot = findUnigitRoot(); // You can define this to find the .unigit root (current or parent)

    // Step 1: Generate new tree and get new tree hash
    createCommitObject(commitPaths, parentHash, author, projectRoot);

    // Step 2: Form commit content
    std::stringstream commitContent;
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    std::string timestamp = std::to_string(duration.count()); // Implement as needed
    commitContent << "tree " << treeHash << "\n";
    if (refCommitHash != "0000000000000000000000000000000000000000") {
        commitContent << "parent " << refCommitHash << "\n";
    }
    commitContent << "author " << author << " " << timestamp << "\n";
    commitContent << "committer " << committer << " " << timestamp << "\n";
    commitContent << "\n";

    std::string rawContent = commitContent.str();

    // Step 3: Add header
    std::string finalContent = "commit " + std::to_string(rawContent.size()) + '\0' + rawContent;

    // Step 4: Hash the final content
    Hasher h = Hasher();
    std::string commitHash = h.hashString(finalContent); // or sha256 if that's what you're using

    // Step 5: Compress and write the commit object
    std::string dir = commitHash.substr(0, 2);
    std::string file = commitHash.substr(2);
    fs::path objectDir = projectRoot / ".unigit" / "object" / dir;
    fs::create_directories(objectDir);
    fs::path commitObjectPath = objectDir / file;

    std::ofstream outFile("temp_commit_object.txt", std::ios::binary);
    outFile << finalContent;
    outFile.close();

    Compressor compressor(4096, 6);
    std::ifstream inFile("temp_commit_object.txt", std::ios::binary);
    std::ofstream outCompressed(commitObjectPath, std::ios::binary);
    compressor.def(inFile, outCompressed);
    inFile.close();
    outCompressed.close();
    fs::remove("temp_commit_object.txt");

    // Step 6: Update ref (HEAD)
    fs::path refFilePath = projectRoot / ".unigit" / "refs" / "heads" / "main"; // we do not have current branch and just main
    std::ofstream refFile(refFilePath);
    refFile << commitHash;
    refFile.close();

    std::cout << "Committed successfully with hash: " << commitHash << std::endl;
}


void CommitObject::createCommitObject(const std::vector<std::string>& inputPaths, std::string &parentHash, std::string &author, fs::path &projectRootfolder) {
    std::cout << "Hello there. Welcome to commit object" << std::endl;
    int CHUNK_SIZE = 4096, LEVEL = 6;
    collectFilesToCommit(inputPaths);

    // tree hash
    // parent hash
    // author

    // description

    // Okay so we have the parent commit hash, we go to the commit object
    // We decompress the commit object
    // We get the tree hash
    // ----> tree hash goes into recursive function <---
    // We go to the tree object, and decompress the tree object
    // We get whole bunch of hashes ---> Now, 
    //                                  Recursively decompress each of these hashes:
    //                                      Base Case: object is blob hash 
    //                                          1. If object blob hash: check if it is asked to be changed. If not, then
    //                                             just return the same objectBlob hash as before
    //                                          2. If tree blob: check if any file inside is asked to be changed. If not,
    //                                             just return same tree hash as before, else recursively go inside each of 
    //                                             the child hashes(which are blobs or tree)

    std::string newCommitHash;

    if (parentHash.empty()) {
        // Handle first commit (no parent)
        std::cout << "First commit detected. Skipping parent commit decompression." << std::endl;

        // Pass empty string as treeHash to signal fresh tree construction
        newCommitHash = TreeObject::recursiveTraverse(filesToCommit, projectRootfolder, "", projectRootfolder);
    } else {
        // Decompress the commit object
        Compressor compressor(CHUNK_SIZE, LEVEL);
        fs::path unigitRefCommitHashObjectPath = projectRootfolder / ".unigit" / "object" /
                                                 parentHash.substr(0, 2) / parentHash.substr(2);
        std::string tempOutputPath = "temp_object.tmp";

        std::cout << "Parent commit hash file path: " << unigitRefCommitHashObjectPath << std::endl;

        if (!fs::exists(unigitRefCommitHashObjectPath)) {
            std::cerr << "Error: Parent commit file not found at path." << std::endl;
            return;
        }

        std::ifstream compressedCommitInput(unigitRefCommitHashObjectPath, std::ios::binary);
        std::ofstream decompressedCommitOutput(tempOutputPath, std::ios::binary);

        int zVal = compressor.inf(compressedCommitInput, decompressedCommitOutput);
        std::cout << "Value of zVal: " << zVal << std::endl;

        compressedCommitInput.close();
        decompressedCommitOutput.close();

        if (zVal != Z_OK) {
            std::cerr << "Error in decompressing commit object" << std::endl;
            return;
        }

        std::ifstream commitFileStream(tempOutputPath, std::ios::binary);
        if (!commitFileStream.is_open()) {
            throw std::runtime_error("Could not open decompressed commit file");
        }

        parseCommitFile(commitFileStream); // this should set `treeHash` and other fields

        // Now we have sha256 for the tree we have to traverse
        newCommitHash = TreeObject::recursiveTraverse(filesToCommit, projectRootfolder, treeHash, projectRootfolder);
    }

    // You can now proceed to write the commit object from newCommitHash, etc.
    std::cout << "New tree hash (commit): " << newCommitHash << std::endl;
    updateHEAD(newCommitHash, projectRootfolder);
}


std::string CommitObject::getParentCommitHash(const std::string& commitHash, const fs::path& projectRootfolder) {
    std::cout << "[DEBUG] Commit hash: " << commitHash << std::endl;

    fs::path objectPath = projectRootfolder / ".unigit" / "object" / commitHash.substr(0, 2) / commitHash.substr(2);
    std::cout << "[DEBUG] Object path: " << objectPath << std::endl;

    // Step 1: Open the compressed commit object
    std::ifstream compressedFile(objectPath, std::ios::binary);
    if (!compressedFile.is_open()) {
        throw std::runtime_error("Failed to open commit object file: " + objectPath.string());
    }

    // Log: Read and show first few bytes
    std::vector<char> compressedPreview(100);
    compressedFile.read(compressedPreview.data(), 100);
    std::streamsize bytesRead = compressedFile.gcount();
    compressedFile.clear(); 
    compressedFile.seekg(0);

    std::cout << "[DEBUG] First " << bytesRead << " bytes of compressed file (hex): ";
    for (std::streamsize i = 0; i < bytesRead; ++i) {
        std::cout << std::hex << (static_cast<unsigned int>(static_cast<unsigned char>(compressedPreview[i]))) << " ";
    }
    std::cout << std::dec << "\n";

    // Step 2: Decompress into a temporary file
    std::ofstream tempOut("temp.commit", std::ios::binary);
    if (!tempOut.is_open()) {
        throw std::runtime_error("Failed to open temporary output file for decompression");
    }

    std::cout << "[DEBUG] Decompressing object..." << std::endl;
    Compressor compressor(4096, 7);
    compressor.inf(compressedFile, tempOut);
    compressedFile.close();
    tempOut.close();
    std::cout << "[DEBUG] Decompression complete." << std::endl;

    // Step 3: Parse the decompressed commit file using ifstream (original logic)
    std::ifstream tempIn("temp.commit", std::ios::binary);
    if (!tempIn.is_open()) {
        throw std::runtime_error("Failed to open temporary commit file for reading");
    }

    std::cout << "[DEBUG] Decompressed file opened successfully for parsing." << std::endl;

    // Log first few bytes of the decompressed file
    std::string preview((std::istreambuf_iterator<char>(tempIn)), std::istreambuf_iterator<char>());
    std::cout << "[DEBUG] Decompressed content preview (first 500 chars):\n" 
              << preview.substr(0, 500) << "\n";
    tempIn.clear();
    tempIn.seekg(0); // rewind before actual parse

    this->parseCommitFile(tempIn);
    tempIn.close();

    std::filesystem::remove("temp.commit");
    std::cout << "[DEBUG] Parent Hash Found: " << this->parentHash << std::endl;

    return this->parentHash;
}



void CommitObject::collectFilesToCommit(const std::vector<std::string>& inputs) {

    for (const std::string& pathStr : inputs) {
        fs::path inputPath(pathStr);
        // std::cout << inputPath.string() << std::endl;
        if (!fs::exists(inputPath)) continue;
    
        if (fs::is_regular_file(inputPath)) {
            filesToCommit.insert(fs::weakly_canonical(inputPath));
        } else if (fs::is_directory(inputPath)) {
            for (auto& p : fs::recursive_directory_iterator(inputPath)) {
                if (fs::is_regular_file(p)) {
                    filesToCommit.insert(fs::weakly_canonical(p.path()));
                }
            }
        }
    }
    std::unordered_set<fs::path> cleanedFilesToCommit;
    for (const auto& file : filesToCommit) {
        fs::path projectRoot = findUnigitRoot();
        fs::path rel = fs::relative(file, projectRoot);

        bool skip = false;
        for (const auto& part : rel) {
            if (part == ".unigit" || part == "..") {
                skip = true;
                break;
            }
        }
        
        if (skip) {
            continue; // skip this file
        }
        cleanedFilesToCommit.insert(file);
    }
    filesToCommit = cleanedFilesToCommit;
    // std::cout << "Files to commit: " << std::endl;
    // for(auto &f : filesToCommit) {
    //     std::cout << f.string() << std::endl;
    // }
    // std::cout << "Files to commit end" << std::endl;
}

void CommitObject::parseCommitFile(std::ifstream &inFile){

    // so our commit object will looks like:
        // tree 0beec7b5ea3f0fdbc95d0dd47f3c5bc275da8a33
        // parent 58f2d539eadd2c48b28065fdfc27635b8f234bbb
        // author UjjawalGusain <ujjawalgusain31@gmail.com> timestamp
        // committer UjjawalGusain <ujjawalgusain31@gmail.com> timestamp
        // --------extra line space to be sure that the next thing is description--------
        // description Add feature X implementation

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    std::string content = buffer.str();
    inFile.close();

    size_t nullPos = content.find('\0');
    if (nullPos == std::string::npos) {
        throw std::runtime_error("Invalid commit object: missing null terminator");
    }

    std::istringstream iss(content.substr(nullPos + 1));
    std::string line;

    while (std::getline(iss, line)) {
        if (line.rfind("tree ", 0) == 0) {
            treeHash = line.substr(5);
        } else if (line.rfind("parent ", 0) == 0) {
            parentHash = line.substr(7);
        } else if (line.rfind("author ", 0) == 0) {
            author = line.substr(7);
        } else if (line.rfind("committer ", 0) == 0) {
            committer = line.substr(10);
        } else if (line.empty()) {
            // Description starts after empty line
            std::ostringstream msg;
            while (std::getline(iss, line)) {
                msg << line << '\n';
            }
            message = msg.str();
            break;
        }
    }
}
