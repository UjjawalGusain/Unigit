#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <string>
#include <stdexcept>
#include <tchar.h>
#include <unordered_set>
#include "../commitObject/commitObject.h"
#include "../utils/utils.h"
#include "json.hpp"
#include "../addObject/addCommand.hpp"
namespace fs = std::filesystem;


void initRepo() {
    std::string author, email;

    fs::path projectPath = fs::current_path();
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

    fs::path projectRootFolder = findProjectRoot();

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

void printStatus() {
    fs::path projectRootFolder = findProjectRoot();
    fs::path watcherFile = projectRootFolder / ".unigit" / "WATCHER";

    // Check if the WATCHER file exists
    if (!fs::exists(watcherFile)) {
        std::cout << "WATCHER file not found at " << watcherFile << std::endl;
        return;
    }

    // Read the content of WATCHER file
    std::ifstream file(watcherFile);
    if (!file.is_open()) {
        std::cout << "Failed to open WATCHER file." << std::endl;
        return;
    }

    nlohmann::json watcherJson;
    try {
        file >> watcherJson;
    } catch (const std::exception& e) {
        std::cout << "Failed to parse WATCHER JSON: " << e.what() << std::endl;
        return;
    }

    std::cout << "Watcher status:" << std::endl;

    bool overloaded = watcherJson.value("overloaded", false);
    std::cout << "Overloaded: " << (overloaded ? "true" : "false") << std::endl;

    auto printArray = [](const nlohmann::json& arr, const std::string& label) {
        std::cout << label << ": ";
        if (arr.is_array() && !arr.empty()) {
            for (size_t i = 0; i < arr.size(); ++i) {
                std::cout << arr[i].get<std::string>();
                if (i != arr.size() - 1) std::cout << ", ";
            }
        } else {
            std::cout << "None";
        }
        std::cout << std::endl;
    };

    printArray(watcherJson["modified"], "Modified");
    printArray(watcherJson["removed"], "Removed");
    printArray(watcherJson["new"], "New");

    std::cout << "Added (file -> hash):" << std::endl;
    if (watcherJson.contains("added") && watcherJson["added"].is_object()) {
        for (auto& [file, hash] : watcherJson["added"].items()) {
            std::cout << "  " << file << " -> " << hash.get<std::string>() << std::endl;
        }
    } else {
        std::cout << "  None" << std::endl;
    }
}



void printInfo() {
    fs::path projectRoot = findProjectRoot();
    fs::path unigitFolder = projectRoot / ".unigit";

    fs::path configFile = unigitFolder / "config.txt";
    fs::path headFile = unigitFolder / "HEAD";

    std::ifstream configStream(configFile);
    if (!configStream.is_open()) {
        std::cout << "Failed to open config.txt" << std::endl;
    } else {
        std::cout << "config.txt content:" << std::endl;
        std::string line;
        while (std::getline(configStream, line)) {
            std::cout << line << std::endl;
        }
        configStream.close();
    }

    std::cout << std::endl;

    std::ifstream headStream(headFile);
    if (!headStream.is_open()) {
        std::cout << "Failed to open HEAD" << std::endl;
    } else {
        std::cout << "HEAD content:" << std::endl;
        std::string headContent;
        std::getline(headStream, headContent);
        std::cout << headContent << std::endl;
        headStream.close();
    }
}

void add(std::vector<std::string> &filenames) {
    std::cout << "Reached add function" << std::endl;
    fs::path projectRootFolder = findProjectRoot();
    fs::path watcherPath = projectRootFolder / ".unigit" / "WATCHER";
    nlohmann::json watcher;

    if (fs::exists(watcherPath)) {
        std::ifstream in(watcherPath);
        in >> watcher;
    } else {
        watcher = {
            {"overloaded", false},
            {"modified",   nlohmann::json::array()},
            {"new",        nlohmann::json::array()},
            {"removed",    nlohmann::json::array()},
            {"added",      nlohmann::json::object()}
        };
    }

    AddCommand addCmd(projectRootFolder, 7);

    for (const auto& filename : filenames) {
        fs::path p = fs::path(filename).lexically_normal();
        fs::path fullPath = projectRootFolder / p;

        if (!fs::exists(fullPath)) {
            std::cerr << "Warning: path does not exist: " << fullPath << "\n";
            continue;
        }

        if (fs::is_regular_file(fullPath)) {
            std::string fileHash = addCmd.hashAndCompressFile(fullPath);
            watcher["added"][p.generic_string()] = fileHash;
            eraseIfExists(watcher["modified"], p.generic_string());
            eraseIfExists(watcher["new"], p.generic_string());
            eraseIfExists(watcher["removed"], p.generic_string());

        } else if (fs::is_directory(fullPath)) {
            addCmd.addBlobsRecursively(fullPath, projectRootFolder, watcher);
        }
    }

    std::ofstream out(watcherPath);
    out << watcher.dump(4);
}


void runCommand(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "No command provided." << std::endl;
        return;
    }

    std::string command = argv[1];
    std::vector<std::string> args(argv + 2, argv + argc);

    if (command == "init") {
        initRepo();
    } else if (command == "commit") {
        handleCommit(args);
    } else if (command == "status") {
        printStatus();
    } else if (command == "info") {
        printInfo();
    } else if (command == "add") {
        if (args.empty()) {
            std::cerr << "No files specified to add." << std::endl;
            return;
        }
        add(args);  
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
    }
}
