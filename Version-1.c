#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "ShellOfHasaan:- "

int execute(char* arglist[]);
char** tokenize(char* cmdline);
char* read_cmd(char*, FILE*);

int main() {
    char *cmdline;
    char **arglist;
    char *prompt = PROMPT;

    while ((cmdline = read_cmd(prompt, stdin)) != NULL) {
        if ((arglist = tokenize(cmdline)) != NULL) {
            execute(arglist);
            // Free allocated memory
            for (int j = 0; j < MAXARGS + 1; j++) {
                free(arglist[j]);
            }
            free(arglist);
            free(cmdline);
        }
    }
    printf("\n");
    return 0;
}

int execute(char* arglist[]) {
    int status;
    pid_t cpid = fork();
    if (cpid == -1) {
        perror("fork failed");
        exit(1);
    } else if (cpid == 0) {
        execvp(arglist[0], arglist);
        perror("Command not found...");
        exit(1);
    } else {
        waitpid(cpid, &status, 0);
        printf("child exited with status %d \n", status >> 8);
        return 0;
    }
}

char** tokenize(char* cmdline) {
    // Allocate memory
    char **arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    if (arglist == NULL) {
        perror("malloc failed");
        exit(1);
    }
    for (int j = 0; j < MAXARGS + 1; j++) {
        arglist[j] = (char*)malloc(sizeof(char) * ARGLEN);
        if (arglist[j] == NULL) {
            perror("malloc failed");
            exit(1);
        }
        bzero(arglist[j], ARGLEN);
    }

    if (cmdline[0] == '\0') // if user has entered nothing and pressed enter key
        return NULL;

    int argnum = 0; // slots used
    char *cp = cmdline; // pos in string
    char *start;
    int len;

    while (*cp != '\0') {
        while (*cp == ' ' || *cp == '\t') // skip leading spaces
            cp++;
        start = cp; // start of the word
        len = 1;
        // find the end of the word
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t'))
            len++;
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }
    arglist[argnum] = NULL;
    return arglist;
}

char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    int c; // input character
    int pos = 0; // position of character in cmdline
    char *cmdline = (char*)malloc(sizeof(char) * MAX_LEN);
    if (cmdline == NULL) {
        perror("malloc failed");
        exit(1);
    }
    while ((c = getc(fp)) != EOF) {
        if (c == '\n')
            break;
        cmdline[pos++] = c;
    }
    // these two lines are added, in case user press ctrl+d to exit the shell
    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL;
    }
    cmdline[pos] = '\0';
    return cmdline;
}