# Tar File Manipulation (`mytar`)

## Description
This project implements `mytar`, a custom version of the `tar` (tape archive) command that creates, extracts, and prints the contents of archive files. The project was created as part of a systems programming course to demonstrate file I/O operations, directory handling, and working with file metadata in C under a UNIX environment.

## Learning Objectives
- Understanding directory entries and file attributes.
- Complex file I/O and file manipulation operations.
- Use of buffered I/O and directory reading system calls.
- Handling file metadata and directory creation.

## Program Specifications

### Name
`mytar` - custom implementation of the `tar` command.

### Synopsis
```bash
mytar [cxtf:] [file] filename

### Examples 
mytar -c -f a.tar a      # Create an archive 'a.tar' containing all files in directory 'a'
mytar -x -f a.tar        # Extract files from the archive 'a.tar'
mytar -t -f a.tar        # Print details of all files in the archive 'a.tar'
