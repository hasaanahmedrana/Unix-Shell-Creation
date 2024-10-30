#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "MyShell:- "

int execute(char *arglist[], int input_fd, int output_fd);
char **tokenize(char *cmdline);
char *read_cmd(char *, FILE *);
void handle_pipes_and_execute(char **arglist);

int main() {
    char *cmdline;
    char **arglist;
    char *prompt = PROMPT;

    while ((cmdline = read_cmd(prompt, stdin)) != NULL) {
        if ((arglist = tokenize(cmdline)) != NULL) {
            handle_pipes_and_execute(arglist);
            for (int j = 0; j < MAXARGS + 1; j++)
                free(arglist[j]);
            free(arglist);
            free(cmdline);
        }
    }
    printf("\n");
    return 0;
}

// Execute a single command with optional redirection
int execute(char *arglist[], int input_fd, int output_fd) {
    pid_t cpid = fork();
    if (cpid == -1) {
        perror("fork() failed");
        exit(1);
    }
    if (cpid == 0) {
        // If input redirection is specified
        if (input_fd != -1) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        // If output redirection is specified
        if (output_fd != -1) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        execvp(arglist[0], arglist);
        perror("!...command not found...!");
        exit(1);
    } else {
        int status;
        waitpid(cpid, &status, 0);
        return status;
    }
}

// Tokenize the command line input into arguments
char **tokenize(char *cmdline) {
    char **arglist = (char **)malloc(sizeof(char *) * (MAXARGS + 1));
    for (int j = 0; j < MAXARGS + 1; j++) {
        arglist[j] = (char *)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[j], ARGLEN);
    }
    if (cmdline[0] == '\0')
        return NULL;

    int argnum = 0;
    char *cp = cmdline;
    char *start;
    int len;

    while (*cp != '\0') {
        while (*cp == ' ' || *cp == '\t')
            cp++;
        start = cp;
        len = 1;
        while (*++cp != '\0' && *cp != ' ' && *cp != '\t')
            len++;
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }
    arglist[argnum] = NULL;
    return arglist;
}

// Read command input
char *read_cmd(char *prompt, FILE *fp) {
    printf("%s", prompt);
    int c;
    int pos = 0;
    char *cmdline = (char *)malloc(sizeof(char) * MAX_LEN);
    while ((c = getc(fp)) != EOF) {
        if (c == '\n')
            break;
        cmdline[pos++] = c;
    }
    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL;
    }
    cmdline[pos] = '\0';
    return cmdline;
}

// Handle pipes and redirection logic
void handle_pipes_and_execute(char **arglist) {
    int input_fd = -1, output_fd = -1;
    int pipefd[2];
    int has_pipe = 0;

    // Look for input/output redirection or pipes
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "<") == 0) {
            // Input redirection
            input_fd = open(arglist[i + 1], O_RDONLY);
            if (input_fd < 0) {
                perror("Failed to open input file");
                return;
            }
            free(arglist[i]);
            arglist[i] = NULL;  // Terminate the argument list here
        } else if (strcmp(arglist[i], ">") == 0) {
            // Output redirection
            output_fd = open(arglist[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd < 0) {
                perror("Failed to open output file");
                return;
            }
            free(arglist[i]);
            arglist[i] = NULL;  // Terminate the argument list here
        } else if (strcmp(arglist[i], "|") == 0) {
            // Pipe handling
            has_pipe = 1;
            if (pipe(pipefd) == -1) {
                perror("pipe() failed");
                return;
            }

            arglist[i] = NULL; // Terminate the argument list here
            execute(arglist, input_fd, pipefd[1]);

            close(pipefd[1]);  // Close the write end
            input_fd = pipefd[0];  // The next command reads from the pipe
            arglist = &arglist[i + 1];  // Move to the next command
            i = -1;  // Reset the loop index
        }
    }

    // Execute the final command with possible redirection
    execute(arglist, input_fd, output_fd);

    if (input_fd != -1) close(input_fd);
    if (output_fd != -1) close(output_fd);
}