#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include "src/commitObject/commitObject.h"
#include "src/blobObject/blobObject.h"
#include "src/commands/commands.hpp"

namespace fs = std::filesystem;


int main(int argc, char* argv[]) {
    runCommand(argc, argv);
    return 0;
}

