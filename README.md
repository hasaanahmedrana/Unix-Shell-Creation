# UNIX Shell Assignment 
---
## Overview
#### Version 1:
Version 1 of a custom UNIX shell provides basic command-line functionalities, 
parsing, and command execution using system calls.
#### Version 2:
Version 2 of a custom UNIX shell extends the basic functionalities by adding support for input/output redirection and piping.
#### Version 3:
Version 3 of a custom UNIX shell further extends the functionalities by adding support for background process handling and signal handling for reaping background processes.
#### Version 4:
Version 4 of a custom UNIX shell includes command history management, allowing users to repeat previous commands using history commands.


---
## Features
#### Version 1:
- **Shell Prompt**: Displays a custom prompt (`ShellOfHasaan:-`) and waits for user input. The prompt may be customized to display additional details, such as the current working directory, using `getcwd()`.
- **Command Execution**: Accepts commands with options and arguments entered by the user, tokenizes them, and utilizes the `fork` and `exec` system calls to execute these commands.
- **Process Management**: The parent process waits for the child process to complete before returning to the prompt for new input.
- **Shell Exit**: Supports exiting the shell using the `<CTRL+D>` command.

#### Version 2:
- **Input/Output Redirection**: Supports input redirection using `<` and output redirection using `>`.
- **Piping**: Supports command piping using `|` to pass the output of one command as input to another.

#### Version 3:
- **Background Process Handling**: Supports running commands in the background using `&`.
- **Signal Handling**: Implements signal handling to reap background processes using `SIGCHLD`.

#### Version 4:
- **Command History**: Maintains a history of the last 10 commands entered by the user.
- **History Commands**: Allows users to repeat previous commands using `!<number>` or `!-1` for the last command.

---

## Known Issues and Bugs
#### Version 1:
- Commands requiring complex input/output redirection or piping are not implemented in this version.
- Background process handling is not yet supported.
#### Version 2:
- Background process handling is not yet supported.
#### Version 3:
- Command history management is not yet supported.

---
## Resources
- **Video Lecture**: [UNIX Shell Development](https://youtu.be/F7oAWvh5J_o?si=_DK3xzetUApoysV-) by Arif Butt
- **Textbook**: *Advanced Programming in the UNIX Environment*, by Richard Stevens
---
## Acknowledgments
Special thanks to the resource person, Arif Butt, for the guidance provided throughout the assignment.

