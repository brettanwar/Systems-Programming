/* THIS CODE WAS MY OWN WORK , IT WAS WRITTEN WITHOUT CONSULTING ANY
SOURCES OUTSIDE OF THOSE APPROVED BY THE INSTRUCTOR . Brett Anwar */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "inodemap.h"
#include <stdint.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <errno.h>
#include <utime.h>
#include <dirent.h>

//used to free map in main
extern char const ** Map;

//list node for creating linked list in create method 
typedef struct ListNode {
    char *filename;             
    struct stat fileStat;       
    struct ListNode *next;   
} ListNode;



// Function declarations
void extractArchive(const char* archiveName);
void listArchive(const char* archiveName);
void processCommandLineArguments(int argc, char** argv);
void createArchive(const char *dirPath, const char *archivePath);

int main(int argc, char** argv) {
    processCommandLineArguments(argc, argv);

    if (Map != NULL) {
        free(Map);
        Map = NULL;
    }
    return 0;
}

//function to handle errors and flags and call necessary functions 
void processCommandLineArguments(int argc, char *argv[]) {
    //flags used to determine what arguments are called
    int c_flag = 0, x_flag = 0, t_flag = 0, f_flag = 0;
    //tarfile stores name after -f
    char *tarfile = NULL;
    //directory stores name for -c
    char *directory = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "cxtf:")) != -1) {
        switch (opt) {
            case 'c':
                c_flag++;
                break;
            case 'x':
                x_flag++;
                break;
            case 't':
                t_flag++;
                break;
            case 'f':
                f_flag++;
                tarfile = optarg;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-cxt] -f [file] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    //error cases 
    if (c_flag + x_flag + t_flag > 1) {
        fprintf(stderr, "Error: Multiple modes specified.\n");
        exit(EXIT_FAILURE);
    }

    if (!c_flag && !x_flag && !t_flag) {
        fprintf(stderr, "Error: No mode specified\n");
        exit(EXIT_FAILURE);
    }

    if (!f_flag) {
        fprintf(stderr, "Error: No tarfile specified\n");
        exit(EXIT_FAILURE);
    }

    if (c_flag && optind < argc) {
        directory = argv[optind];
    }

    // Checks existence and type of the directory (for -c option)
    if (c_flag) {
        if (!directory) {
            fprintf(stderr, "Error: No directory target specified\n");
            exit(EXIT_FAILURE);
        }

        struct stat statbuf;
        if (stat(directory, &statbuf) != 0) {
            fprintf(stderr, "Specified target(\"%s\") does not exist.\n", directory);
            exit(EXIT_FAILURE);
        }

        if (!S_ISDIR(statbuf.st_mode)) {
            fprintf(stderr, "Specified target(\"%s\") is not a directory.\n", directory);
            exit(EXIT_FAILURE);
        }
    }
    if (t_flag){
        listArchive(tarfile);
    }
    if (x_flag){
        extractArchive(tarfile);
    }
    if (c_flag){
        createArchive(directory, tarfile);
    }
}

//helper method to build linked list for create function 
void populateListFromDirectory(const char *basePath, ListNode **headRef, ListNode **currentRef) {
    DIR *dir = opendir(basePath);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Ignore '.' / '..' 
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char fullPath[PATH_MAX];
        //concantenates strings and adds /
        strcpy(fullPath, basePath);
        strcat(fullPath, "/");
        strcat(fullPath, entry->d_name);

        struct stat fileStat;
        if (lstat(fullPath, &fileStat) == -1) {
            perror("Failed to get file stat");
            continue;
        }

        // Create a new list node
        ListNode *newNode = malloc(sizeof(ListNode));
        if (newNode == NULL) {
            perror("Failed to allocate memory for ListNode");
            continue;
        }
        newNode->filename = strdup(fullPath); // Duplicate string to avoid overwriting
        newNode->fileStat = fileStat;
        newNode->next = NULL;

        // Adds new node to linked list
        if (*headRef == NULL) {
            *headRef = newNode;
        } else {
            (*currentRef)->next = newNode;
        }
        *currentRef = newNode;

       //recursive call for directories iterate through its contents
        if (S_ISDIR(fileStat.st_mode)) {
            populateListFromDirectory(fullPath, headRef, currentRef);
        }
    }

    closedir(dir);
}

//function that creates tarfile based on given directory
void createArchive(const char *dirPath, const char *archivePath) {
    ListNode *head = NULL;
    ListNode *current = NULL;

    // Create list node for directory
    struct stat dirStat;
    if (lstat(dirPath, &dirStat) == -1) {
        perror("Failed to get file statistics for the initial directory");
        return;
    }
    //creates an itial node to pass in so that the directory it self is also included 
    ListNode *dirNode = malloc(sizeof(ListNode));
    if (dirNode == NULL) {
        perror("Failed to allocate memory for the initial directory ListNode");
        return;
    }
    dirNode->filename = strdup(dirPath);
    dirNode->fileStat = dirStat;
    dirNode->next = NULL;
    head = current = dirNode;

    // Populates linked list with files and directories
    populateListFromDirectory(dirPath, &head, &current);

    FILE *tarFile = fopen(archivePath, "wb");
    if (!tarFile) {
        perror("Failed to open archive file for writing");
        return;
    }

    //writes in magic num to top of tar file
    uint32_t magicNumber = 0x7261746D;
    fwrite(&magicNumber, sizeof(magicNumber), 1, tarFile);

    ListNode *temp = head;
    while (temp != NULL) {
        if (S_ISLNK(temp->fileStat.st_mode)) {
            // Skip soft links
            temp = temp->next;
            continue;
        }

        uint64_t inodeNumber = temp->fileStat.st_ino;
        //checks for hard link 
        const char *existingFileName = get_inode(inodeNumber);

        uint32_t nameLength = strlen(temp->filename);
        fwrite(&inodeNumber, sizeof(inodeNumber), 1, tarFile);
        fwrite(&nameLength, sizeof(nameLength), 1, tarFile);
        fwrite(temp->filename, nameLength, 1, tarFile);

        //checks if is hard link or not
        if (existingFileName == NULL) { 
            set_inode(inodeNumber, temp->filename); 

            fwrite(&temp->fileStat.st_mode, sizeof(temp->fileStat.st_mode), 1, tarFile);
            fwrite(&temp->fileStat.st_mtime, sizeof(temp->fileStat.st_mtime), 1, tarFile);
            //checks if regular file
            if (S_ISREG(temp->fileStat.st_mode)) {
                uint64_t fileSize = temp->fileStat.st_size;
                fwrite(&fileSize, sizeof(fileSize), 1, tarFile);

                FILE *file = fopen(temp->filename, "rb");
                if (file) {
                    char buffer[1024];
                    size_t bytesRead;
                    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                        fwrite(buffer, 1, bytesRead, tarFile);
                    }
                    fclose(file);
                } else {
                    perror("Failed to open file for reading");
                }
            }
        }
        temp = temp->next; 
    }
    fclose(tarFile);
}

//function to extract archive 
void extractArchive(const char* archiveName) {
    uint32_t magicNumber;
    uint64_t inodeNumber, fileModTime, fileSize;
    uint32_t fileNameLength, fileMode;
    char *fileName;
    FILE *archiveFile = fopen(archiveName, "r");
    //read and check magic number 
    fread(&magicNumber, sizeof(magicNumber), 1, archiveFile);

    if (magicNumber != 0x7261746D) {
        fprintf(stderr, "Error: Bad magic number (%d), should be 1918989421.\n", magicNumber);
        exit(EXIT_FAILURE);
    }
    //reads in each file/dir/link  
    while (fread(&inodeNumber, sizeof(inodeNumber), 1, archiveFile) == 1) {
        fread(&fileNameLength, sizeof(fileNameLength), 1, archiveFile);
        fileName = malloc(fileNameLength + 1);
        fread(fileName, fileNameLength, 1, archiveFile);
        fileName[fileNameLength] = '\0'; // Null-terminate the filename
        fread(&fileMode, sizeof(fileMode), 1, archiveFile);
        fread(&fileModTime, sizeof(fileModTime), 1, archiveFile);

        //checks if hard link
        const char *existingFileName = get_inode(inodeNumber);
        
        if (existingFileName != NULL) {
            link(existingFileName, fileName);
        }
        //checks if is directory and creates if so
        else if (S_ISDIR(fileMode)) {
            if (mkdir(fileName, fileMode) < 0 && errno != EEXIST) {
                perror("Error creating directory");
            }
            //checks if regular file 
        } else if (S_ISREG(fileMode)) {
            FILE *outFile = fopen(fileName, "w");
            if (!outFile) {
                perror("Error creating file");
                free(fileName);
                continue;
            }

            if (fread(&fileSize, sizeof(fileSize), 1, archiveFile) != 1) {
                perror("Error reading file size");
                fclose(outFile);
                free(fileName);
                continue;
            }
            //creates buffer 
            char *buffer = malloc(fileSize);
            if (buffer && fread(buffer, 1, fileSize, archiveFile) == fileSize) {
                fwrite(buffer, 1, fileSize, outFile);
            } else {
                perror("Error reading file content");
            }
            fclose(outFile);
        }
        if (existingFileName == NULL){
            set_inode(inodeNumber, fileName);
        }

        // Setting file metadata, checks if file or dir then updates mod times
        struct stat statbuf;
        if (S_ISDIR(fileMode) || S_ISREG(fileMode)){
            if (stat(fileName, &statbuf) == 0) {
            struct utimbuf new_times;
            new_times.actime = fileModTime;
            new_times.modtime = fileModTime;
            if (chmod(fileName, fileMode) != 0 || utime(fileName, &new_times) != 0) {
                perror("Error setting file metadata");
            }
        }
        }
    }
    fclose(archiveFile);
}

//method to print the contents of tar file
void listArchive(const char* archiveName) {
    uint32_t magicNumber;
    uint64_t inodeNumber, fileModTime, fileSize;
    uint32_t fileNameLength, fileMode;
    char *fileName;
    FILE *archiveFile = fopen(archiveName, "r");


    // Read and validate the magic number
    if (fread(&magicNumber, sizeof(magicNumber), 1, archiveFile) != 1) {
        perror("Error reading magic number");
        exit(EXIT_FAILURE);
    }

    if (magicNumber != 0x7261746D) {
        fprintf(stderr, "Error: Bad magic number (%d), should be 1918989421.\n", magicNumber);
        exit(EXIT_FAILURE);
    }

    // Loop through each file in the archive
    while (fread(&inodeNumber, sizeof(inodeNumber), 1, archiveFile) == 1) {
        
        fread(&fileNameLength, sizeof(fileNameLength), 1, archiveFile);

        //allocates space for filename and reads it in
        fileName = malloc(fileNameLength + 1);
        fread(fileName, 1, fileNameLength, archiveFile);
        fileName[fileNameLength] = '\0'; //end of file character

        //checks for hard link
        if(get_inode(inodeNumber)) {           
                printf("%s/ -- inode: %llu\n", fileName, (unsigned long long)inodeNumber);
                free(fileName);
                continue;
        } else {
                set_inode(inodeNumber, fileName);
        }

        //gets mode of file or dir
        fread(&fileMode, sizeof(fileMode), 1, archiveFile);

        //gets mod time of file
        fread(&fileModTime, sizeof(fileModTime), 1, archiveFile);

        //handing for regular files 
        if (S_ISREG(fileMode)) { 
            if (fread(&fileSize, sizeof(fileSize), 1, archiveFile) != 1) {
                perror("Error reading file size");
                free(fileName);
                break;
            }
            // Skip over file content
            if (fseek(archiveFile, fileSize, SEEK_CUR) != 0) {
                perror("Error seeking through file content");
                free(fileName);
                break;
            }
            //checks for executable bits using bitwise 
            if((fileMode % 01000) & (S_IXUSR | S_IXGRP | S_IXOTH)){
                    printf("%s* -- inode: %llu, mode: %o, mtime: %llu, size: %llu\n", fileName, (unsigned long long)inodeNumber, fileMode % 01000, (unsigned long long)fileModTime, (unsigned long long)fileSize);
            } else {
                    printf("%s -- inode: %llu, mode: %o, mtime: %llu, size: %llu\n", fileName, (long long unsigned int) inodeNumber, fileMode % 01000, (long long unsigned int) fileModTime, (long long unsigned int) fileSize);
            }
            //handing for directories 
        } else if (S_ISDIR(fileMode)) { 
            printf("%s/ -- inode: %llu, mode %o, mtime: %llu\n", fileName, (unsigned long long)inodeNumber, fileMode % 01000, (unsigned long long)fileModTime);
        }
        free(fileName);
    }

    fclose(archiveFile);
}
