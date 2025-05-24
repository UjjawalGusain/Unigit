#include "src/blobObject/blobObject.h"
#include "src/commands/commands.hpp"
#include "src/commitObject/commitObject.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    runCommand(argc, argv);
    return 0;
}
