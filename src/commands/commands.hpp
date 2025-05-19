#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <stdexcept>
#include <tchar.h>
#include <windows.h> 
#include "../commitObject/commitObject.h"
#include "../utils/utils.h"

namespace fs = std::filesystem;


void initRepo() {
    std::string author, email;

    // Use current directory directly as project root
    TCHAR buffer[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, buffer);
    fs::path projectPath = fs::path(buffer);
    std::string projectName = projectPath.filename().string();

    fs::path unigitDir = projectPath / ".unigit";
    if (fs::exists(unigitDir)) {
        std::cout << "Repository already initialized at: " << unigitDir << "\n";
        return;
    }

    fs::create_directory(unigitDir);
    fs::create_directory(unigitDir / "object");

    std::cout << "Enter author name: ";
    std::getline(std::cin, author);

    std::cout << "Enter email: ";
    std::getline(std::cin, email);

    std::ofstream configFile(unigitDir / "config.txt");
    configFile << "author=" << author << "\n";
    configFile << "email=" << email << "\n";
    configFile << "project=" << projectName << "\n";
    configFile.close();

    std::cout << "Initialized empty UniGit repository for '" << projectName << "' in: " << unigitDir << "\n";
}



void handleCommit(const std::vector<std::string>& args) {
    if (args.size() < 4) {
        std::cerr << "Usage: unigit commit <branch> <author> <description>" << std::endl;
        return;
    }

    std::string branch = args[1];
    std::string author = args[2];
    std::string description = args[3];

    // fs::path cwd = fs::current_path();
    fs::path projectRootFolder = findProjectRoot();
    // fs::path projectRootFolder = cwd;

    std::vector<std::string> inputPaths;
    for (const auto& entry : fs::recursive_directory_iterator(projectRootFolder)) {
        if (entry.is_regular_file() && entry.path().string().find(".unigit") == std::string::npos) {
            inputPaths.push_back(entry.path().string());
        }
    }

    std::string parentHash;
    parentHash = getCurrentCommitHash(projectRootFolder);

    CommitObject commit(branch, projectRootFolder);
    commit.createCommitObject(inputPaths, parentHash, author, projectRootFolder);

    std::cout << "Commit created." << std::endl;
}

void runCommand(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "No command provided." << std::endl;
        return;
    }

    std::string command = argv[1];
    std::vector<std::string> args(argv + 1, argv + argc);

    if (command == "init") {
        initRepo();
    } else if (command == "commit") {
        handleCommit(args);
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
    }
}
