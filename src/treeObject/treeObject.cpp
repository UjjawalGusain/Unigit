#include <iostream>
#include <filesystem>
#include <unordered_set>
#include <vector>
#include "utils.h"
#include "blobObject.h"
#include "treeObject.h"
#include "compress.h"

namespace fs = std::filesystem;
int CHUNK_SIZE = 4096, LEVEL = 6;

std::string TreeObject::recursiveTraverse(std::unordered_set<fs::path> &filesToCommit, fs::path &projectRootfolder,std::string &hash, fs::path &currentFilePath) {
    // base case: if hash is blobObject hash, check if we have to change it.
    fs::path objectPath = projectRootfolder / ".unigit" / "object" / hash.substr(0, 2) / hash.substr(2);
    std::string objectType = getObjectType(objectPath, hash);
    if(objectType == "blob") {
        std::string newHash;
        if(filesToCommit.count(currentFilePath)) {
            // change it
            fs::path cwd = fs::current_path();
            int level = 6;

            BlobObject blob(cwd, currentFilePath.string(), level);
            blob.write();
            newHash = blob.getHash(); 
        } else {
            newHash = hash; 
        }
        return newHash;
    }

        // if object type is tree, you have to traverse it recursively.
        // But first, you have to traverse through the contents of that tree dir
    else if(objectType == "tree") {
        // Tree object will look like:
            // <type> <filesize>/0/n
            // blob file.txt  abcd1234...
            // blob readme.md efgh5678...
            // blob readme.md efgh5678...
            // basically => <type> <name> <hash>

            Compressor compressor(CHUNK_SIZE, LEVEL);      
                  
            std::vector<bool> shouldTraversePath;
            const fs::path &temp_path = currentFilePath + 
        
    }


}
