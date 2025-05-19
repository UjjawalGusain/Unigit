#ifndef INIT_HPP
#define INIT_HPP
#include <iostream>
#include <filesystem>
#include <fstream>
#include <windows.h>

namespace fs = std::filesystem;

void initRepo() {
    std::string author, email, projectName;
    std::cout << "Enter project name: ";
    std::cin >> projectName;

    fs::path projectPath;
    if (projectName == ".") {
        TCHAR buffer[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, buffer);
        projectPath = fs::path(buffer);
        projectName = projectPath.filename().string();
    } else {
        projectPath = fs::path(projectName);
        if (!fs::exists(projectPath)) {
            fs::create_directory(projectPath);
        }
    }

    fs::path unigitDir = projectPath / ".unigit";
    if (fs::exists(unigitDir)) {
        std::cout << "Repository already initialized at: " << unigitDir << "\n";
        return;
    }

    fs::create_directory(unigitDir);
    fs::create_directory(unigitDir / "object");

    std::cout << "Enter author name: ";
    std::cin.ignore(); 
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

#endif