# PPM Image Transformations

## Description
This project, developed as a part of a systems programming course, is a command-line tool written in C for manipulating Portable Pixel Map (PPM) image files. The tool, `ppmcvt`, allows for a variety of image transformations including conversion to different formats (PBM, PGM), channel isolation, sepia effects, mirroring, thumbnail creation, and tiling. This utility operates within a Linux environment, providing a hands-on approach to systems programming concepts like memory allocation, command-line parsing, and file I/O.

## Learning Objectives
- Develop, compile, run, and test C programs in a Linux environment.
- Navigate Linux command lines effectively.
- Utilize advanced features of C such as structs, pointers, and memory allocation.

## Program Specification
- **Name**: `ppmcvt`
- **Synopsis**: `ppmcvt [OPTIONS] FILE`
- **Options**:
  - `-b`: Convert to PBM (default).
  - `-g`: Convert to PGM using specified max grayscale value.
  - `-i`: Isolate specified RGB channel.
  - `-r`: Remove specified RGB channel.
  - `-s`: Apply sepia transformation.
  - `-m`: Mirror image vertically.
  - `-t`: Create a thumbnail with scaling factor.
  - `-n`: Tile thumbnails based on scaling factor.
  - `-o`: Specify output file.
- **Exit Status**: 0 on success, 1 on failure.

## Installation
This program is intended to be compiled and run on a Linux system with GCC installed. To compile the program, navigate to the directory containing the source files and run the provided `makefile`.

## Usage
To use `ppmcvt`, navigate to the directory containing the compiled program and run it with the desired options. Examples of common operations include:

```bash
ppmcvt -o out.pbm in.ppm # Convert in.ppm to out.pbm
ppmcvt -g 16 -o out.pgm in.ppm # Convert to PGM with max grayscale 16
ppmcvt -s -o out.ppm in.ppm # Apply sepia transformation
