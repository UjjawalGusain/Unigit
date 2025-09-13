# Unigit - A Simple Version Control System
<div align="center">
  <img src="https://github.com/user-attachments/assets/b9db9b49-1d93-41cc-8744-12667f797b57" alt="unigit_src_image" width="512" height="512" />
</div>


Welcome to **Unigit**, a lightweight version control system inspired by Git, designed to help you manage and track changes to source code and files in your projects. Whether you're working solo or experimenting with version control, Unigit provides a simple yet powerful way to keep your work organized.

## Table of Contents
- [Unigit - A Simple Version Control System](#unigit---a-simple-version-control-system)
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
  - [Features](#features)
  - [Architecture Diagram](#architecture-diagram)
  - [Demo](#demo)
  - [Dependencies](#dependencies)
  - [Building Instructions](#building-instructions)
  - [Basic Usage Example](#basic-usage-example)
  - [How It Works](#how-it-works)
  - [Future Work and Limitations](#future-work-and-limitations)
  - [Contributing](#contributing)
  - [License](#license)

## Introduction
Unigit is a custom-built version control system that mirrors some of Git's core functionality while keeping things simple and approachable. It allows you to initialize repositories, stage and commit changes, view project history, and revert to previous states—all with a straightforward command-line interface. Perfect for learning about version control or managing small projects, Unigit is your friendly companion for tracking changes.

## Features
Unigit comes with a set of core commands to manage your projects effectively:

- **`unigit init`**: Initializes a new Unigit repository in the current directory, creating the `.unigit` folder and necessary configuration files.
- **`unigit add <file1> <file2> ...`**: Stages specified files for the next commit, tracking them in the staging area.
- **`unigit commit <branch> <author> <description>`**: Saves staged changes as a commit, identified by a unique SHA256 hash.
- **`unigit status`**: Displays the current state of your working directory, showing modified, new, removed, or staged files.
- **`unigit logs`**: Shows the commit history for the current branch, including commit hashes, authors, dates, and messages.
- **`unigit info`**: Provides repository details, such as the author, project name, and current `HEAD` commit.
- **`unigit cat <hash>`**: Retrieves and displays the content of a Unigit object (e.g., blob or commit) using its SHA256 hash.
- **`unigit checkout <commit_hash>`**: Restores the working directory to the state of a specified commit.

## Architecture Diagram

![image](https://github.com/user-attachments/assets/8d55e60e-1dcd-44fb-bf5d-a0b7eb15ee4d)

## Demo
Link for the demo: https://www.linkedin.com/posts/ujjawal-gusain_building-unigit-an-os-enhanced-version-control-activity-7334961240454156288-yswZ

## Dependencies
To build and run Unigit, you’ll need the following libraries:

- **zlib**: For data compression.
- **nlohmann/json**: For handling JSON data in the staging area and configuration.
- **SHA256**: A custom or third-party implementation for generating content hashes.
- **BS_thread_pool**: For efficient parallel processing, such as when adding multiple files.

Ensure these dependencies are installed or available in your build environment.

## Building Instructions
Unigit is written in C++ and includes a `makefile` to streamline the build process. Follow these steps to compile and set up Unigit:

1. **Compile the Project**:
   - Open a terminal and navigate to the Unigit project’s root directory.
   - Run the following command to build the project:
     ```bash
     make
     ```
     Alternatively, use `make compress` (an alias for the default build target).

2. **Locate the Executable**:
   - The compiled executable is placed at `C:/tools/Unigit/bin/unigit.exe` by default. The `make` command will attempt to create this directory if it doesn’t exist.
   - To change the output location, modify the `OUT` variable in the `makefile`.

3. **Add to PATH (Optional, Recommended)**:
   - To run `unigit` from any directory, add its location to your system’s PATH environment variable.

   **Windows**:
   - Search for "Edit the system environment variables" in the Start menu.
   - Click "Environment Variables...".
   - Find the `Path` variable under "System variables" (or "User variables").
   - Click "Edit...", then "New", and add the path to the directory containing `unigit.exe` (e.g., `C:\tools\Unigit\bin`).
   - Click "OK" to save and restart your terminal for the changes to take effect.

   **Linux/macOS**:
   - Locate the directory containing the `unigit` executable.
   - Open your shell configuration file (e.g., `~/.bashrc`, `~/.zshrc`, or `~/.profile`).
   - Add the following line, replacing `/path/to/unigit_directory` with the actual path:
     ```bash
     export PATH="/path/to/unigit_directory:$PATH"
     ```
   - Save the file and run `source ~/.bashrc` (or the appropriate file) to apply changes, or open a new terminal.

## Basic Usage Example
Here’s a quick example to get you started with Unigit:

```bash
# Initialize a new repository
unigit init

# Create or modify files
echo "Hello World" > file1.txt
echo "Another file" > file2.txt

# Stage the files
unigit add file1.txt file2.txt

# Commit the changes
unigit commit main "Your Name" "Initial commit"

# Check the status
unigit status

# View commit logs
unigit logs

# Make further changes
echo "More changes" >> file1.txt
unigit add file1.txt
unigit commit main "Your Name" "Updated file1"

# Revert to a previous commit
# (Replace <previous_commit_hash> with a hash from 'unigit logs')
unigit checkout <previous_commit_hash>
```

## How It Works
Unigit operates with a few key components that make version control seamless:

- **`.unigit` Directory**: The core of your repository, storing metadata, commit history, and configuration.
- **Object Store (`.unigit/object/`)**: Stores file content (blobs) and commit details, compressed and identified by SHA256 hashes. The first two characters of each hash are used as subdirectory names for organization.
- **`WATCHER` File (`.unigit/WATCHER`)**: A JSON file that serves as the staging area, tracking files added with `unigit add`. It’s also used by `unigit status` and `unigit logs`.
- **`config.txt` (`.unigit/config.txt`)**: Stores repository settings, like author name and email, used in commits.
- **`HEAD` File (`.unigit/HEAD`)**: Tracks the SHA256 hash of the latest commit, representing the current state of your project.

## Future Work and Limitations
Unigit is a work in progress with some planned improvements and current limitations:

- The `unigit checkout <commit_hash> <file_name>` command, which would allow checking out a single file from a commit, is not yet implemented.
- Additional features, such as branching and merging, may be added in future releases to enhance functionality.

## Contributing
We welcome contributions to Unigit! If you’d like to add features, fix bugs, or improve documentation:
1. Fork the repository.
2. Create a feature branch (`git checkout -b feature/your-feature`).
3. Commit your changes and push to your fork.
4. Submit a pull request with a clear description of your changes.

Please ensure your code follows the project’s style and includes appropriate tests.

## License
Unigit is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

Happy version controlling with Unigit! 
