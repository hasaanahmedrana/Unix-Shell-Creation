#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "ShellOfHasaan:- "
#define MAX_HISTORY 10
#define MAX_ALIASES 10

typedef struct {
    char name[ARGLEN];
    char command[MAX_LEN];
} Alias;

int execute(char* cmd[], char* history[], int* history_count, Alias aliases[], int* alias_count);
char** tokenize(char* cmdline);
char* read_cmd(char*, FILE*);
void add_to_history(char* cmd, char* history[], int* history_count);
void set_alias(char* name, char* command, Alias aliases[], int* alias_count);
char* get_alias(char* name, Alias aliases[], int alias_count);

// Global job counter to number background jobs
int job_counter = 1;

// Signal handler to reap background processes
void handle_sigchld(int sig) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);  // Reap all child processes
    errno = saved_errno;
}

int main() {
    // Set up signal handler to handle SIGCHLD for background process reaping
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("sigaction");
        exit(1);
    }

    char *cmdline;
    char** cmd;
    char* history[MAX_HISTORY] = { NULL }; // Command history array
    int history_count = 0; // Count of commands in history
    Alias aliases[MAX_ALIASES] = { { "", "" } }; // Alias array
    int alias_count = 0; // Count of aliases

    while((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        // Check for command history repeat
        if (cmdline[0] == '!') {
            if (strcmp(cmdline, "!-1") == 0) {
                // Repeat last command
                if (history_count > 0) {
                    cmdline = strdup(history[history_count - 1]);
                    printf("Repeating command: %s\n", cmdline);
                } else {
                    printf("No commands in history.\n");
                    continue; // Skip to the next iteration
                }
            } else {
                int cmd_num = atoi(&cmdline[1]);
                if (cmd_num >= 1 && cmd_num <= history_count) {
                    cmdline = strdup(history[cmd_num - 1]); // Get the command from history
                    printf("Repeating command: %s\n", cmdline);
                } else {
                    printf("Invalid command number\n");
                    continue; // Skip to next loop iteration
                }
            }
        } else {
            add_to_history(cmdline, history, &history_count); // Add command to history
        }

        if((cmd = tokenize(cmdline)) != NULL) {
            // Check for alias command
            if (strcmp(cmd[0], "alias") == 0) {
                if (cmd[1] != NULL) {
                    char* equal_sign = strchr(cmd[1], '=');
                    if (equal_sign != NULL) {
                        *equal_sign = '\0';
                        set_alias(cmd[1], equal_sign + 1, aliases, &alias_count);
                    } else {
                        printf("Invalid alias format. Use alias name=command\n");
                    }
                } else {
                    printf("alias: expected argument\n");
                }
            } else {
                // Check if the command is an alias
                char* alias_command = get_alias(cmd[0], aliases, alias_count);
                if (alias_command != NULL) {
                    free(cmdline);
                    cmdline = strdup(alias_command);
                    cmd = tokenize(cmdline);
                }
                execute(cmd, history, &history_count, aliases, &alias_count);
            }
            for(int j = 0; j < MAXARGS + 1; j++) free(cmd[j]);
            free(cmd);
            free(cmdline);
        }
    }
    printf("\n");
    // Free history commands
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }
    return 0;
}

void add_to_history(char* cmd, char* history[], int* history_count) {
    if (*history_count < MAX_HISTORY) {
        history[*history_count] = strdup(cmd); // Duplicate and store command
        (*history_count)++;
    } else {
        // Shift history left and overwrite the oldest command
        free(history[0]); // Free the oldest command
        for (int i = 1; i < MAX_HISTORY; i++) {
            history[i - 1] = history[i]; // Shift left
        }
        history[MAX_HISTORY - 1] = strdup(cmd); // Add new command
    }
}

void set_alias(char* name, char* command, Alias aliases[], int* alias_count) {
    for (int i = 0; i < *alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            strcpy(aliases[i].command, command);
            return;
        }
    }
    if (*alias_count < MAX_ALIASES) {
        strcpy(aliases[*alias_count].name, name);
        strcpy(aliases[*alias_count].command, command);
        (*alias_count)++;
    } else {
        printf("Alias limit reached.\n");
    }
}

char* get_alias(char* name, Alias aliases[], int alias_count) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            return aliases[i].command;
        }
    }
    return NULL;
}

int execute(char* cmd[], char* history[], int* history_count, Alias aliases[], int* alias_count) {
    int background = 0;
    int in = -1, out = -1;
    int num_cmds = 0;
    char* command[MAXARGS][MAXARGS];
    int i, j = 0;
    pid_t pid;  // Declare pid here to capture the last command’s pid for background jobs

    // Handle built-in commands
    if (strcmp(cmd[0], "cd") == 0) {
        if (cmd[1] == NULL) {
            fprintf(stderr, "cd: expected argument\n");
        } else {
            if (chdir(cmd[1]) != 0) {
                perror("chdir failed");
            }
        }
        return 1;
    }
    if (strcmp(cmd[0], "exit") == 0) {
        exit(0);
    }

    // Parse command line for background, redirection, and pipes
    for (i = 0; cmd[i] != NULL; i++) {
        if (strcmp(cmd[i], "&") == 0) {
            background = 1;
            cmd[i] = NULL;
            break;
        } else if (strcmp(cmd[i], "<") == 0) {
            in = open(cmd[i + 1], O_RDONLY);
            if (in < 0) {
                perror("Failed to open input file");
                return -1;
            }
            cmd[i] = NULL;
            i++;
        } else if (strcmp(cmd[i], ">") == 0) {
            out = open(cmd[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (out < 0) {
                perror("Failed to open output file");
                return -1;
            }
            cmd[i] = NULL;
            i++;
        } else if (strcmp(cmd[i], "|") == 0) {
            command[num_cmds][j] = NULL;
            num_cmds++;
            j = 0;
        } else {
            command[num_cmds][j++] = cmd[i];
        }
    }
    command[num_cmds][j] = NULL;
    num_cmds++;

    // Create pipes for each command in the pipeline
    int pipefd[2 * (num_cmds - 1)];
    for (i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipefd + i * 2) == -1) {
            perror("Pipe failed");
            exit(1);
        }
    }

    // Execute each command in the pipeline
    for (i = 0; i < num_cmds; i++) {
        pid = fork();
        if (pid == 0) {  // Child process
            // Redirect input for the first command
            if (i == 0 && in != -1) {
                dup2(in, STDIN_FILENO);
                close(in);
            }
            // Redirect output for the last command
            if (i == num_cmds - 1 && out != -1) {
                dup2(out, STDOUT_FILENO);
                close(out);
            }

            // Redirect input from previous pipe
            if (i > 0) {
                dup2(pipefd[(i - 1) * 2], STDIN_FILENO);
            }
            // Redirect output to next pipe
            if (i < num_cmds - 1) {
                dup2(pipefd[i * 2 + 1], STDOUT_FILENO);
            }

            // Close all pipe file descriptors in the child
            for (j = 0; j < 2 * (num_cmds - 1); j++) {
                close(pipefd[j]);
            }

            // Execute the command
            execvp(command[i][0], command[i]);
            perror("Command Not Found");
            exit(1);
        } else if (pid < 0) {
            perror("Fork failed");
            return -1;
        }
    }

    // Close all pipe file descriptors in the parent
    for (i = 0; i < 2 * (num_cmds - 1); i++) {
        close(pipefd[i]);
    }

    // If in the foreground, wait for all commands to complete
    if (!background) {
        for (i = 0; i < num_cmds; i++) {
            wait(NULL);
        }
    } else {
        printf("[%d] %d\n", job_counter++, pid);  // Print job number and PID for the last background process
    }

    return 0;
}

// Tokenize function to split command into arguments
char** tokenize(char* cmdline) {
    char** cmd = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    if (cmd == NULL) {
        perror("malloc failed");
        exit(1);
    }
    for(int j = 0; j < MAXARGS + 1; j++) {
        cmd[j] = (char*)malloc(sizeof(char) * ARGLEN);
        if (cmd[j] == NULL) {
            perror("malloc failed");
            exit(1);
        }
        bzero(cmd[j], ARGLEN);
    }
    if(cmdline[0] == '\0') return NULL;

    int argnum = 0;
    char* cp = cmdline;
    char* start;
    int len;

    while(*cp != '\0') {
        while(*cp == ' ' || *cp == '\t') cp++;
        start = cp;
        len = 1;
        while(*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) len++;
        strncpy(cmd[argnum], start, len);
        cmd[argnum][len] = '\0';
        argnum++;
    }
    cmd[argnum] = NULL;
    return cmd;
}

// Read command input from user
char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    int c, pos = 0;
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    if (cmdline == NULL) {
        perror("malloc failed");
        exit(1);
    }
    while((c = getc(fp)) != EOF) {
        if(c == '\n') break;
        cmdline[pos++] = c;
    }
    if(c == EOF && pos == 0) {
        free(cmdline);
        return NULL;
    }
    cmdline[pos] = '\0';
    return cmdline;
}