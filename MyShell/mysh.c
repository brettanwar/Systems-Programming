/* THIS CODE WAS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING ANY
SOURCES OUTSIDE OF THOSE APPROVED BY THE INSTRUCTOR. Brett Anwar */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>      
#include "tokens.h"
#include <sys/stat.h>  
#include <errno.h>

#define MAX_INPUT_LENGTH 1024
#define MAX_PIPES 100 //incease if testing > 100

// Function to find the next pipe "|" in an array of tokens
int find_next_pipe(char **tokens, int start) {
    for (int i = start; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "|") == 0) {
            return i;
        }
    }
    return -1;
}

// Function to check if a token is a shell command
int is_shell_command(const char *token) {
    const char *shell_commands[] = {"ls", "cd", "echo", "pwd", "rm", "mv", "cp", "touch", "cat"};
    for (int i = 0; i < sizeof(shell_commands) / sizeof(shell_commands[0]); i++) {
        if (strcmp(token, shell_commands[i]) == 0) {
            return 1; // It's a shell command
        }
    }
    return 0; // It's not a shell command
}

// Function to close unused pipes in a pipe array
void close_unused_pipes(int pipes[][2], int pipe_count, int current) {
    for (int i = 0; i < pipe_count; i++) {
        if (i != current) {
            close(pipes[i][1]);
        }
        if (i != current - 1) {
            close(pipes[i][0]);
        }
    }
}

// Function to close all pipes in a pipe array
void close_all_pipes(int pipes[][2], int pipe_count) {
    for (int i = 0; i < pipe_count; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}


int main(int argc, char *argv[]) {
    char input[MAX_INPUT_LENGTH];
    char *prompt = "mysh: ";
    int should_run = 1;

    // Custom prompt handling
    if (argc == 2) {
        if (strcmp(argv[1], "-") == 0) {
            prompt = "";
        } else {
            char *newPrompt = malloc(strlen(argv[1]) + 3);
            if (newPrompt == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
            strcpy(newPrompt, argv[1]);
            strcat(newPrompt, ": ");
            prompt = newPrompt;
        }
    }
    
    while (should_run) {
        
        printf("%s", prompt);
        int pipe_count = 0;
        int pipes[MAX_PIPES][2];
        int in_redirection_count = 0;
        int is_pipe_present = 0;

        if (fgets(input, MAX_INPUT_LENGTH, stdin) == NULL) {
            if (feof(stdin)) {
                break;
            }
            continue;
        }

        // Check for input length exceeding limit after pressing Enter
        if (strlen(input) == MAX_INPUT_LENGTH - 1 && input[MAX_INPUT_LENGTH - 2] != '\n') {
            fprintf(stderr, "Error: Input line too long.\n");
            // Clear the input buffer
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
            continue;
        }

        input[strcspn(input, "\n")] = 0;
        char **tokens = get_tokens(input);

        if (tokens[0] != NULL && strcmp(tokens[0], "exit") == 0) {
            should_run = 0;
            continue;
        }

        for (int i = 0; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], "|") == 0) {
                if (tokens[i + 1] == NULL) {
                    fprintf(stderr, "Error: Invalid null command after pipe.\n");
                    continue; // Skip the rest of the processing and prompt for new input
                }
                if (pipe_count == MAX_PIPES) {
                    fprintf(stderr, "Error: Too many pipes.\n");
                    exit(EXIT_FAILURE);
                }
                if (pipe(pipes[pipe_count]) == -1) {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
                pipe_count++;
            }
        }

        
        // Check for incorrect usage of '&'
        int found_ampersand = 0;
        for (int i = 0; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], "&") == 0) {
                if (tokens[i + 1] != NULL) { // '&' is not the last token
                    fprintf(stderr, "Error: \"&\" must be the last token on command line.\n");
                    found_ampersand = 1;
                    break;
                }
            }
        }

        if (found_ampersand) {
            free_tokens(tokens);
            continue; // Skip the rest of the processing and prompt for new input
        }

        //check for pipe and < 
        for (int i = 0; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], "|") == 0) {
                is_pipe_present = 1;
            } else if (strcmp(tokens[i], "<") == 0) {
                in_redirection_count++;
            }
        }

        if (is_pipe_present && in_redirection_count > 0) {
            fprintf(stderr, "Error: Ambiguous input redirection.\n");
            free_tokens(tokens);
            continue; // Skip the rest of the processing and prompt for new input
        }

        // Check for ambiguous redirection
        int out_redirection_count = 0;
        in_redirection_count = 0;
        for (int i = 0; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], ">>") == 0) {
                if (tokens[i + 1] == NULL) {
                    fprintf(stderr, "Error: Missing filename for output redirection.\n");
                    continue; // Skip the rest of the processing and prompt for new input
                }
                out_redirection_count++;
            } else if (strcmp(tokens[i], "<") == 0) {
                in_redirection_count++;
            }
        }

        if (out_redirection_count > 1) {
            fprintf(stderr, "Error: Ambiguous output redirection.\n");
            continue;
        }

        if (in_redirection_count > 1) {
            fprintf(stderr, "Error: Ambiguous input redirection.\n");
            continue;
        }

        int background = 0;

        int num_tokens = 0;
        for (num_tokens = 0; tokens[num_tokens] != NULL; num_tokens++);

        if (num_tokens > 0 && strcmp(tokens[num_tokens - 1], "&") == 0) {
            if (tokens[0] == NULL || tokens[num_tokens - 2] == NULL) {
                    fprintf(stderr, "Error: Invalid null command before \"&\".\n");
                    free_tokens(tokens);
                    continue; // Skip the rest of the processing and prompt for new input
                }
            background = 1;
            free(tokens[num_tokens - 1]);
            tokens[num_tokens - 1] = NULL;
        }

        int command_start = 0;
        for (int i = 0; i <= pipe_count; i++) {
            int command_end = (i == pipe_count) ? num_tokens : find_next_pipe(tokens, command_start);
            tokens[command_end] = NULL;

            pid_t pid = fork();
            if (pid == 0) { // Child process

                // Handle pipes
                if (i < pipe_count) {
                    if (tokens[command_start] == NULL) {
                    fprintf(stderr, "Error: Invalid null command.\n");
                    continue; // Skip the rest of the processing and prompt for new input
                }
                    dup2(pipes[i][1], STDOUT_FILENO);
                    close(pipes[i][1]);
                }
                if (i > 0) {
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                    close(pipes[i - 1][0]);
                }
                close_unused_pipes(pipes, pipe_count, i);

                // Handle I/O redirection
                for (int i = 0; tokens[i] != NULL; i++) {
                    int fd;
                    if (strcmp(tokens[i], ">") == 0) {
                        if (tokens[i + 1] == NULL) {
                            fprintf(stderr, "Error: Missing filename for input redirection.\n");
                            exit(EXIT_FAILURE); // Skip the rest of the processing and prompt for new input
                        }
                        if (tokens[i + 1] == NULL || is_shell_command(tokens[i + 1])) {
                            fprintf(stderr, "Error: Invalid null command.\n");
                            exit(EXIT_FAILURE); // Skip the rest of the processing and prompt for new input
                        }
                        if ((fd = open(tokens[i + 1], O_WRONLY | O_CREAT | O_EXCL, 0644)) == -1) {
                            fprintf(stderr, "Error: open(\"%s\"): File exists\n", tokens[i + 1]);
                            exit(EXIT_FAILURE);
                        }
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                        tokens[i] = NULL;
                        break;
                    } else if (strcmp(tokens[i], ">>") == 0) {
                        if ((fd = open(tokens[i + 1], O_WRONLY | O_APPEND | O_CREAT, 0644)) == -1) {
                            // perror("Error opening file for output appending");
                            // exit(EXIT_FAILURE);
                        }
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                        tokens[i] = NULL;
                        break;
                    } else if (strcmp(tokens[i], "<") == 0) {
                        if ((fd = open(tokens[i + 1], O_RDONLY)) == -1) {
                            perror("Error opening file for input redirection");
                            exit(EXIT_FAILURE);
                        }
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                        tokens[i] = NULL;
                        break;
                    }
                }
                 if (execvp(tokens[command_start], &tokens[command_start]) == -1) {
                    fprintf(stderr, "%s: No such file or directory\n", tokens[command_start]);
                    exit(EXIT_FAILURE);
                }
                // Error handling
                exit(EXIT_FAILURE);
            } else if (pid > 0) { // Parent process
                command_start = command_end + 1;
            } else {
                perror("fork");
                exit(EXIT_FAILURE);
            }
        }
        close_all_pipes(pipes, pipe_count);
        if (!background) {
            //handles zombie processess
            for (int i = 0; i <= pipe_count; i++) {
                while(wait(NULL) > 0){
                    continue;
                };
            }
        }
        free_tokens(tokens);
    }

    if (argc == 2 && strcmp(argv[1], "-") !=0) {
        free(prompt);
    }

    return 0;
}
