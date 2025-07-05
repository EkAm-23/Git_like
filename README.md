# Git_like

## 📦 Project Overview

This project is an implementation of a basic version control system, inspired by Git, written in **C++**. The purpose of this project is to build a foundational understanding of how version control systems like Git work internally — including features like **commits, branching, staging, merging,** and more.

## 🚀 Key Features

- **Repository Creation**: Initialize a new repository in any directory.
- **Staging Area (Index)**: Add files to the staging area before committing changes.
- **Commit System**: Track snapshots of different repository versions.
- **Branching**: Create and manage branches for parallel development.
- **Checkout**: Switch between branches or revert to specific commits.
- **Log**: View the history of commits.
- **Merge**: Combine two branches together.
- **Conflict Resolution**: Handle conflicts during merges.

## ⚙️ Usage

Run this command while in the project directory:

```bash
g++ src/cli.cpp src/merge.cpp src/staging.cpp src/commit.cpp src/branch.cpp src/repository.cpp -o gitlike -lssl -lcrypto -lz
```

## Project structure

├── src                  # Source files\
│   ├── zstr
│   ├── cli.cpp          # Entry point for the program, handles command management\
│   ├── repository.cpp   # Handles repository creation and management\
│   ├── commit.cpp       # Manages commit creation and history\
│   ├── branch.cpp       # Handles branch creation and management\
│   ├── staging.cpp      # Handles the index file creation and staging\
│   └── merge.cpp        # Handles merging of branches and conflict resolution\
│\
├── include              # Header files\
│   ├── repository.h\
│   ├── commit.h\
│   ├── branch.h\
│   ├── staging.h\
│   └── merge.h\
│\
├── .gitlike/            # Data storage for the version control (generated on init)\
│   ├── objects/         # Stores commit objects\
│   ├── refs/            # Stores branch references\
│   ├── index            # Stores the staged file paths and last staged hash values\
│   └── HEAD             # Points to the current branch/commit

## 📝 Notes

-The system uses OpenSSL EVP APIs for SHA-1 hashing and zstr for zlib-based compression.\
-Tree and commit objects are content-addressable, allowing for deduplication and secure versioning.\
-Branches are tracked via .gitlike/refs/heads/<branchname>, and the HEAD file stores the current reference pointer.\
-The project replicates Git’s internal object model, using blobs, trees, commits, and a DAG-based history structure.
