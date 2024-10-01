Sure! Here's a `README.md` based on the description you provided for your SnapTracker project:

---

# SnapTracker

**SnapTracker** is a C-based project designed to help you track your files by generating snapshot files that store essential metadata about each file. The project is compatible with both macOS and Linux and includes a bash script to identify and isolate potentially dangerous files. This project was developed as an individual university project.

## Table of Contents

- [Features](#features)
- [How It Works](#how-it-works)
- [Usage](#usage)
- [Compilation](#compilation)
- [Running SnapTracker](#running-snaptracker)
- [Example Commands](#example-commands)
- [Bash Script](#bash-script)
- [Project Structure](#project-structure)

## Features

- Create snapshot files for specified files and directories.
- Metadata in snapshots includes:
  - File path
  - File name
  - File size
  - Last modified time
  - Inode
  - File permissions
- Efficient performance through the use of processes and pipes.
- Customizable output location for snapshot files.
- Isolation of potentially dangerous files based on specific criteria.

## How It Works

SnapTracker allows you to monitor files or directories by generating a snapshot file for each specified file or directory. Each snapshot file contains metadata that enables you to track changes over time, functioning similarly to version control systems like Git, but focused on file metadata.

### Example Snapshot File

```
Path: directory/file
Name: file
Size: 12 bytes
Last Modified: 2024-04-07 06:57:08
Inode: 50475123
Permissions: rw-r--r--
```

## Usage

To use SnapTracker, you'll first need to compile the C source files using `gcc`. It is important to note that macOS uses Apple Clang, so there are different source files for macOS (`main.c`) and Linux (`main_linux.c`).

The project also includes a bash script to help identify potentially dangerous files based on certain criteria.

## Compilation

To compile SnapTracker, use the following command, making sure to replace `your_source_file.c` with the appropriate source file for your operating system:

```bash
gcc -s isolated_directory example-directory your_source_file.c
```

For macOS, use `main.c`, and for Linux, use `main_linux.c`.

## Running SnapTracker

Run SnapTracker with the following arguments:

- `-o output_directory`: Specifies the output directory where all snapshots will be stored. This argument must be the first if present.
- `-s isolated_directory`: Specifies the directory where potentially dangerous files will be moved. This argument should follow the output argument if both are present.
- Up to 10 directories can be specified for generating snapshots.

### Example Command

```bash
./snaptracker -o output -s isolated dir1 dir2 dir3
```

In this example, SnapTracker will generate snapshot files for `dir1`, `dir2`, and `dir3`, store the snapshots in the `output` directory, and move any potentially dangerous files to the `isolated` directory.

## Example Commands

```bash
# Create snapshots for directories with isolated dangerous files
./snaptracker -o snapshots -s quarantine dir1 dir2
```

## Bash Script

The project includes a bash script that identifies potentially dangerous files using the following criteria:

- Files containing non-ASCII characters.
- Files containing certain keywords (e.g., `dangerous`, `malicious`, `corrupted`).
- Files with a high density of characters over a small number of lines.

The script moves identified files to the directory specified with the `-s` argument (the isolated directory).
