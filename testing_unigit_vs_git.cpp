#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <sstream>

namespace fs = std::filesystem;

void remove_readonly_and_delete(const fs::path& path) {
    if (!fs::exists(path)) return;
    for (auto& p : fs::recursive_directory_iterator(path)) {
        std::error_code ec;
        fs::permissions(p, fs::perms::owner_all, fs::perm_options::add, ec);
    }
    std::error_code ec;
    fs::remove_all(path, ec);
    if (ec) {
        std::cerr << "Failed to remove " << path << ": " << ec.message() << std::endl;
    }
}

void remove_gitignore(const fs::path& dir_path) {
    for (auto& p : fs::recursive_directory_iterator(dir_path)) {
        if (p.path().filename() == ".gitignore") {
            std::error_code ec;
            fs::remove(p.path(), ec);
            if (ec) {
                std::cerr << "Failed to remove " << p.path() << ": " << ec.message() << std::endl;
            }
        }
    }
}

double execute_and_time(const std::string& command) {
    auto start = std::chrono::high_resolution_clock::now();
    int result = std::system(command.c_str());
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << end.time_since_epoch().count() << std::endl;
    if (result != 0) {
        std::cerr << "Command failed: " << command << std::endl;
    }
    std::chrono::duration<double> elapsed = end - start;
    return elapsed.count();
}


void test_repo(const fs::path& repo_path, std::ofstream& results) {
    std::cout << "Testing repo: " << repo_path << std::endl;

    // remove_gitignore(repo_path);

    std::system(("cd \"" + repo_path.string() + "\" && git init").c_str());


    double git_add_time = execute_and_time("cd \"" + repo_path.string() + "\" && git add .");

    double git_commit_time = execute_and_time("cd \"" + repo_path.string() + "\" && git commit -m \"first commit\" --allow-empty");


    results << repo_path.filename().string() << " - git add: " << git_add_time << "s, git commit: " << git_commit_time << "s\n";
    results.flush();

    remove_readonly_and_delete(repo_path / ".git");

    std::string init_command = "cd \"" + repo_path.string() + "\" && echo \"Ujjawal Gusain\" && echo \"ujjawalgusain31@gmail.com\" | unigit init";
    std::system(init_command.c_str());

    // Run unigit add and time it
    double unigit_add_time = execute_and_time("cd \"" + repo_path.string() + "\" && unigit add .");

    // Run unigit commit and time it
    double unigit_commit_time = execute_and_time("cd \"" + repo_path.string() + "\" && unigit commit main UjjawalGusain \"first commit\"");

    // Log unigit results
    results << repo_path.filename().string() << " - unigit add: " << unigit_add_time << "s, unigit commit: " << unigit_commit_time << "s\n";
    results << "---------------------------------------------\n";
    results.flush();

    // Delete .unigit directory to clean up for next run
    remove_readonly_and_delete(repo_path / ".unigit");
}

int main() {
    fs::path root_path = "testing";
    std::ofstream results("testing/test_results_wsl.txt", std::ios::app);

    if (!results) {
        std::cerr << "Failed to open test_results.txt for writing.\n";
        return 1;
    }

    for (auto& dir : fs::directory_iterator(root_path)) {
        if (fs::is_directory(dir)) {
            for (auto& project : fs::directory_iterator(dir)) {
                if (fs::is_directory(project)) {
                    test_repo(project.path(), results);
                }
            }
        }
    }

    results.close();
    std::cout << "Testing completed. Results saved to testing/test_results.txt\n";
    return 0;
}
