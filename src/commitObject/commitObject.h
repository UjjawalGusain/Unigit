#ifndef COMMITOBJECT_H
#define COMMITOBJECT_H
#include <iostream>
#include <vector>
#include <unordered_set>
#include "fileObject.h"
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class CommitObject : public FileObject {

    public:
        CommitObject(const fs::path& root, const std::string& source, int compressionLevel = 6);
        CommitObject(const std::string& branch, fs::path& cwd);
        std::string getType() const override { return "commit"; }

        std::string parentHash;
        std::string refCommitHash;
        std::string treeHash;
        std::string author;
        std::string committer;
        std::string message;
        std::unordered_set<fs::path> filesToCommit;
    
        CommitObject(const std::string& branch, fs::path& cwd);
    
        void createCommitObject(const std::vector<std::string>& inputPaths, std::string &parentHash, std::string &author, std::string &description, fs::path &projectRootfolder);
    
    private:
    
        void collectFilesToCommit(const std::vector<std::string>& inputs);
        void parseCommitFile(std::ifstream &inFile);
};

#endif  // COMMITOBJECT_H
