[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/wtw9xmrw)
# Shell

This project is a comprehensive user-friendly shell interface that enhances process control, user interaction, and error handling mechanisms. Works with external and internal commands, background processing, and I/O redirection. 

## Group Members
- **Rebecca Anestad**: rba20a@fsu.edu
- **Olivia Judah**: ogj21@fsu.edu
- **Amelia Sieg**: ats20b@fsu.edu
## Division of Labor

### Part 1: Prompt
- **Responsibilities**: [Print the prompt when it should be printed, including user, machine and pwd]
- **Assigned to**: Rebecca Anestad, Olivia Judah, Amelia Sieg

### Part 2: Environment Variables
- **Responsibilities**: [Ensure that all environment variables are with getenv()]
- **Assigned to**: Rebecca Anestad, Olivia Judah, Amelia Sieg

### Part 3: Tilde Expansion
- **Responsibilities**: [Check if a ~ appears in the tokens. If it does, expand it to the home variable which is the home directory.]
- **Assigned to**: Rebecca Anestad, Olivia Judah, Amelia Sieg

### Part 4: $PATH Search
- **Responsibilities**: [Expand the path variables, which is multiple directories, and go through each directory. If the correct path directory is found, then return it.]
- **Assigned to**: Rebecca Anestad, Olivia Judah

### Part 5: External Command Execution
- **Responsibilities**: [Fork to create a child process and then use path search to find the correct command. If it is found, use execv to execute the correct command. Other wise return error. ]
- **Assigned to**: Rebecca Anestad, Olivia Judah, Amelia Sieg

### Part 6: I/O Redirection
- **Responsibilities**: [Replace the keyboard with input from the specified file and redirect output to a designated file.]
- **Assigned to**: Rebecca Anestad, Amelia Sieg

### Part 7: Piping
- **Responsibilities**: [Simultaneous execution of multiple commands, changing the input and output of each respectivly so that the output of one command is the input of the next command]
- **Assigned to**: Rebecca Anestad

### Part 8: Background Processing
- **Responsibilities**: [Allows the shell to execute external commands without waiting for their execution. Also keeps track of their status.]
- **Assigned to**: Olivia Judah, Amelia Sieg

### Part 9: Internal Command Execution
- **Responsibilities**: [Built-in functions such as echo, cd, exit, and jobs.]
- **Assigned to**: Rebecca Anestad, Olivia Judah, Amelia Sieg

### Part 10: External Timeout Executable
- **Responsibilities**: [Created a new project, mytimeout, that executes the command passed in and waits the specified amount of time before terminating.]
- **Assigned to**: Rebecca Anestad

### Extra Credit
- **Responsibilities**: [Shell-ception executes our shell from within a running shell process repeatedly]
- **Assigned to**: Rebecca Anestad, Olivia Judah, Amelia Sieg

## File Listing
```
shell/
│
├── src/
│ └── lexer.c
| └── mytimeout/
|   └── mytimeout.c
|
├── include/
│ └── lexer.h
│
├── README.md
└── Makefile
```
## How to Compile & Execute
To clean work space, run "make clean"
To compile all executables run "make" 
To run the shell executable run "make run"
### Requirements
- **Compiler**: `gcc

### Compilation
```bash
make
```
This will build the executable in bin
### Execution
```bash
make run
```
This will run the program

## Bugs
- **Bug 1**: N/A

## Extra Credit
- **Extra Credit 1**: [Shell-ception]
- **Extra Credit 2**: [Supports piping and I/O redirection in single command]
- **Extra Credit 3**: [Extra Credit Option]

## Considerations
-prints "/bin/sleep: invalid time interval ‘&’ Try '/bin/sleep --help' for more information." when we do the sleep 3 & command however it still works
-active background processes switch from running to done after one command
-potential memory leaks 
-Olivia is submitting two days after due date because of granted extension with doctor's note
