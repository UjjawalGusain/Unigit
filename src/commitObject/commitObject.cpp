#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include "compress.h"
#include "header.h"
#include "hasher.h"
#include "utils.h" // gets us getObjectType function
#include "blobObject.h"
#include "treeObject.h"
#include "fileObject.h"
#include "commitObject.h"
namespace fs = std::filesystem;
int CHUNK_SIZE = 4096, LEVEL = 6;

CommitObject::CommitObject(const fs::path& root, const std::string& source, int compressionLevel)
    : FileObject(root, source, compressionLevel) {
    // No additional initialization is required for CommitObject.
}


CommitObject::CommitObject(const std::string& branch, fs::path& cwd) {
    fs::path refCommitHashFilePath = cwd / ".unigit" / "refs" / "heads" / branch;

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
}


void CommitObject::createCommitObject(const std::vector<std::string>& inputPaths, std::string &parentHash, std::string &author, std::string &description, fs::path &projectRootfolder) {

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
                                        // Recursively decompress each of these hashes:
                                        //  Base Case: object is blob hash 
                                            // 1. If object blob hash: check if it is asked to be changed. If not, then
                                            //    just return the same objectBlob hash as before
                                            // 2. If tree blob: check if any file inside is asked to be changed. If not,
                                            //    just return same tree hash as before, else recursively go inside each of 
                                            //    the child hashes(which are blobs or tree)

        
    // Decompress the commit object
    Compressor compressor(CHUNK_SIZE, LEVEL);
    fs::path unigitRefCommitHashObjectPath = projectRootfolder / ".unigit" / "object" / refCommitHash.substr(0,2) / refCommitHash.substr(2);
    std::string tempOutputPath = "temp_object.tmp";
    std::ifstream compressedCommitInput(unigitRefCommitHashObjectPath);
    std::ofstream decompressedCommitOutput(tempOutputPath, std::ios::binary);
    int zVal = compressor.inf(compressedCommitInput, decompressedCommitOutput);

    if(zVal != Z_OK) {
        std::cerr << "Error in decompressing object" << std::endl;
        return;
    }
    compressedCommitInput.close();
    decompressedCommitOutput.close();

    std::ifstream commitFileStream(tempOutputPath, std::ios::binary);
    if (!commitFileStream.is_open()) {
        throw std::runtime_error("Could not open decompressed commit file");
    }
        
    parseCommitFile(commitFileStream);

    // Now we have sha256 for the tree we have to traverse

    std::string newCommitHash = TreeObject::recursiveTraverse(filesToCommit, projectRootfolder, treeHash, projectRootfolder);
    
}



    
void CommitObject::collectFilesToCommit(const std::vector<std::string>& inputs) {
    
    for (const std::string& pathStr : inputs) {
        fs::path inputPath(pathStr);
    
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
