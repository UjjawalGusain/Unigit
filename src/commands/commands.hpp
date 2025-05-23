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
#include "../checkoutCommand/checkoutCommand.hpp"
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

void updateWatcher(nlohmann::json& watcher) {
    if (!watcher.contains("added") || !watcher["added"].is_object())
        return;

    for (auto it = watcher["added"].begin(); it != watcher["added"].end(); ++it) {
        std::string filePath = it.key();

        for (const std::string& category : {"modified", "new", "removed"}) {
            if (watcher.contains(category) && watcher[category].is_array()) {
                auto& arr = watcher[category];
                arr.erase(std::remove(arr.begin(), arr.end(), filePath), arr.end());
            }
        }
    }

    watcher["added"] = nlohmann::json::object();
}

void handleCommit(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cerr << "Usage: unigit commit <branch> <author> <description>" << std::endl;
        return;
    }

    std::string branch = args[0];
    std::string author = args[1];
    std::string description = args[2];

    fs::path projectRootFolder = findProjectRoot();

    fs::path watcherFile = projectRootFolder / ".unigit" / "WATCHER";

    if (!fs::exists(watcherFile)) {
        std::cerr << "No files staged. Use unigit add <filename1> <filename2>..." << std::endl;
        return;
    }

    std::ifstream file(watcherFile);
    if (!file.is_open()) {
        std::cout << "Failed to open WATCHER file." << std::endl;
        return;
    }

    nlohmann::json watcher;
    try {
        file >> watcher;
    } catch (const std::exception& e) {
        std::cout << "Failed to parse WATCHER JSON: " << e.what() << std::endl;
        return;
    }

    if (!watcher.contains("index") || watcher["index"].empty()) {
        std::cerr << "No files staged. Use unigit add <filename1> <filename2>..." << std::endl;
        return;
    }

    std::string parentHash = getCurrentCommitHash(projectRootFolder);
    if (parentHash.empty()) {
        std::cout << "Performing initial commit..." << std::endl;
    }

    std::string commitHash = CommitObject::commit(parentHash, watcher, projectRootFolder, projectRootFolder, author, description);

    if (!commitHash.empty()) {
        nlohmann::json logEntry = {
            {"hash", commitHash},
            {"branch", branch},
            {"author", author},
            {"description", description},
            {"timestamp", std::time(nullptr)}
        };

        if (!watcher.contains("logs") || !watcher["logs"].is_array()) {
            watcher["logs"] = nlohmann::json::array();
        }

        watcher["logs"].push_back(logEntry);
    }

    std::cout << "Commit hash: " << commitHash << std::endl;

    if (commitHash.empty()) {
        std::cerr << "commit hash is empty" << std::endl;
    } else {
        updateHEAD(commitHash, projectRootFolder);
        updateWatcher(watcher);
    }

    std::ofstream watcherOut(watcherFile);
    if (watcherOut.is_open()) {
        watcherOut << watcher.dump(4);
        watcherOut.close();
    } else {
        std::cerr << "Failed to update WATCHER file." << std::endl;
    }

    std::cout << "Commit created." << std::endl;

    fs::path tempBlob = projectRootFolder / "temp_commit_input.blob";
    if (fs::exists(tempBlob)) {
        fs::remove(tempBlob);
    }
}


void printStatus() {
    fs::path projectRootFolder = findProjectRoot();
    fs::path watcherFile = projectRootFolder / ".unigit" / "WATCHER";

    if (!fs::exists(watcherFile)) {
        std::cout << "WATCHER file not found at " << watcherFile << std::endl;
        return;
    }

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

void handleLogs() {
    fs::path projectRootFolder = findProjectRoot();
    fs::path watcherFile = projectRootFolder / ".unigit" / "WATCHER";

    if (!fs::exists(watcherFile)) {
        std::cerr << "No commits found." << std::endl;
        return;
    }

    std::ifstream file(watcherFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open WATCHER file." << std::endl;
        return;
    }

    nlohmann::json watcher;
    try {
        file >> watcher;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse WATCHER JSON: " << e.what() << std::endl;
        return;
    }

    if (!watcher.contains("logs") || !watcher["logs"].is_array()) {
        std::cerr << "No commit logs found." << std::endl;
        return;
    }

    for (const auto& log : watcher["logs"]) {
        std::time_t t = log.value("timestamp", 0);
        std::cout << "Commit: " << log.value("hash", "") << std::endl;
        std::cout << "Author: " << log.value("author", "") << std::endl;
        std::cout << "Branch: " << log.value("branch", "") << std::endl;
        std::cout << "Message: " << log.value("description", "") << std::endl;
        std::cout << "Date: " << std::asctime(std::localtime(&t));
        std::cout << "-----------------------------" << std::endl;
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
            {"added",      nlohmann::json::object()},
            {"index",      nlohmann::json::object()}
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
            std::string fileHash = addCmd.hashFile(fullPath);  // <-- just hash, no compress here!
            std::cout << "File staged: " << fullPath << std::endl;


            fs::path objectPath = projectRootFolder / ".unigit" / "object" / fileHash.substr(0, 2) / fileHash.substr(2);

            bool alreadyTracked = watcher["added"].contains(p.generic_string()) &&
                                  watcher["added"][p.generic_string()] == fileHash;
            
            bool objectExists = fs::exists(objectPath);

            if (!objectExists) {
                addCmd.hashAndCompressFile(fullPath);
            }

            if (!alreadyTracked) {
                watcher["added"][p.generic_string()] = fileHash;
                watcher["index"][p.generic_string()] = fileHash;
                eraseIfExists(watcher["modified"], p.generic_string());
                eraseIfExists(watcher["new"], p.generic_string());
                eraseIfExists(watcher["removed"], p.generic_string());
            }
            
        } else if (fs::is_directory(fullPath)) {
            addCmd.addBlobsRecursively(fullPath, projectRootFolder, watcher);
        }
    }

    std::ofstream out(watcherPath);
    out << watcher.dump(4);

    fs::path tempBlob = projectRootFolder / "temp_commit_input.blob";
    if (fs::exists(tempBlob)) {
        fs::remove(tempBlob);
    }
}

void cat(std::vector<std::string> &args) {
    if((int)args.size() != 1) {
        std::cerr << "cat command format: unigit cat hash" << std::endl;
        return;
    }

    std::string hash = args[0];

    if((int)hash.size() < 4) {
        std::cerr << "Atleast 4 character hash is required" << std::endl;
        return;
    }

    fs::path projectRootFolder = findProjectRoot();

    fs::path folderPath = projectRootFolder / ".unigit" / "object" / hash.substr(0, 2);
    if(!fs::exists(folderPath)) {
        std::cerr << "Object folder does not have this hash" << std::endl;
        return;
    }

    std::string remainingHash = hash.substr(2);
    std::vector<fs::path> matches;
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.rfind(remainingHash, 0) == 0) { 
                matches.push_back(entry.path());
            }
        }
    }

    if (matches.empty()) {
        std::cerr << "No object found with the given hash" << std::endl;
        return;
    } else if (matches.size() > 1) {
        std::cerr << "Ambiguous hash: multiple objects match this hash" << std::endl;
        return;
    }

    std::ifstream file(matches[0], std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open object file" << std::endl;
        return;
    }

    std::stringstream buffer;
    std::ostream& output = buffer; 

    Compressor compressor(4096, 7);
    int result = compressor.inf(file, reinterpret_cast<std::ofstream&>(output));
    if (result != Z_OK) {
        std::cerr << "Failed to decompress and display object (error code: " << result << ")" << std::endl;
        return;
    }

    std::cout << buffer.str();
}

void checkout(std::vector<std::string> &args) {
    fs::path projectRootFolder = findProjectRoot();

    if (args.size() > 2 || args.size() <= 0) {
        std::cerr << "Format of checkout: \n";
        std::cerr << "unigit checkout <commit_hash>\n";
        std::cerr << "unigit checkout <commit_hash> <file_name>\n";
        return;
    }

    std::string commitHash = args[0];
    fs::path commitObjectPath = projectRootFolder / ".unigit" / "object" / commitHash.substr(0, 2) / commitHash.substr(2);

    if (!fs::exists(commitObjectPath)) {
        std::cerr << "Commit object does not exist" << std::endl;
        return;
    }

    if (args.size() == 1) {
        CheckoutCommand::restoreCommit(commitHash, projectRootFolder);
        std::cout << "Workspace updated to commit " << commitHash << std::endl;
    } else {
        std::cerr << "unigit checkout <commit_hash> <file_name> is not implemented yet.\n";
    }
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
    } else if (command == "cat") {
        cat(args);
    } else if (command == "checkout") {
        checkout(args);
    } else if (command == "logs") {
        handleLogs();
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
    }
}
