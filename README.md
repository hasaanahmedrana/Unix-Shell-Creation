# UNIX Shell Assignment - Version 1
---
## Overview
#### Version 1:
Version 1 of a custom UNIX shell provides basic command-line functionalities, 
parsing, and command execution using system calls. 
---
## Features
#### Version 1:
- **Shell Prompt**: Displays a custom prompt (`PUCITshell:-`) and waits for user input. The prompt may be customized to display additional details, such as the current working directory, using `getcwd()`.
- **Command Execution**: Accepts commands with options and arguments entered by the user, tokenizes them, and utilizes the `fork` and `exec` system calls to execute these commands.
- **Process Management**: The parent process waits for the child process to complete before returning to the prompt for new input.
- **Shell Exit**: Supports exiting the shell using the `<CTRL+D>` command.

---

## Known Issues and Bugs
- Commands requiring complex input/output redirection or piping are not implemented in this version.
- Background process handling is not yet supported.
---
## Resources
- **Video Lecture**: [UNIX Shell Development](https://youtu.be/F7oAWvh5J_o?si=_DK3xzetUApoysV-) by Arif Butt
- **Textbook**: *Advanced Programming in the UNIX Environment*, by Richard Stevens
---
## Acknowledgments
Special thanks to the resource person, Arif Butt, for the guidance provided throughout the assignment.

