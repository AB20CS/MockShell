# Mock Shell

## Approach
This mock shell supports three commands - Echo, Forx and List. In my implementation, each command is handled by a separate function. Throughout the program, the file to which the outputs will be written is managed using file descriptors.

#### Echo
The `write` function is used to write the given string to the desired file.

#### Forx
This is command is implemented by setting up a pipe between the parent process and child process (created by forking after the `forx` command is called). The desired program is executed in the child process using `execvp`. Then, output of the executed program is redirected from `stdout` (using `dup2`) and is sent to the parent process via the pipe.

#### List
The list command is implemented using recursion where `interp` (or its analogue `interp_rec`) is called as each command is received (for more information about each of these functions, please consult the the **Handling Redirection of `stdout`** subsection and **Function Overview** section below). When either of these functions are called, the program iterates over the commands in the list and executes them sequentially.

#### Handling Redirection of `stdout`
There are two cases to handle:
1. the command given is not part of a list
2. the command is part of a list

_Case 1:_ In this case, a `redir_stdout` value of `NULL` means an output to `stdout`. If `redir_stdout` is not `NULL`, the filename specified is the output file. The redirections are made by keeping note of the desired output file. The `interp` function is used to handle this case.

_Case 2:_ In this case, the output file depends on the output file specified for the full list. If the `redir_stdout` value for the command is `NULL`, the output file for the command is the same as the output file specified for the list. Otherwise, the output file is the file specified in `redir_stdout` value for the command itself. The `interp_rec` function is used to handle this case to ensure that the output file specified for the full list influences the output of the individual commands in the list.


## Function Overview
#### Primary Functions
| Function | Description |
| --- | --- |
| `interp(const struct cmd *c)` | Handles commands that are not part of a list, where `c` is the struct holding the command. the Refer to _Case 1_ in **Handling Redirection of `stdout`** subsection above. |
| `interp_rec(const struct cmd *c)` | Handles commands that are part of a list, where `c` is the struct holding the command. the Refer to _Case 2_ in **Handling Redirection of `stdout`** subsection above. |

#### Helper Functions
| Function | Description |
| --- | --- |
| `echo(int fd, const struct *c)` | Executes the echo command, where `fd` is the file descriptor for the output file and `c` is the struct holding the command. |
| `forx(int fd, const struct *c)` | Executes the forx command, where `fd` is the file descriptor for the output file and `c` is the struct holding the command. |
| `list(int fd, const struct *c)` | Executes the list command, where `fd` is the file descriptor for the output file and `c` is the struct holding the command. |

## Running the Program

1. Ensure `Makefile` is in the same directory as `byos.h` and `byos.c`.
2. `Makefile` has currently been configured to handle inputs specified in a file called `simple-test.c`. Please substitute all occurrences of `simple-test` in the `Makefile` with the name excluding the `.c` extension of the C file containing the input commands  (e.g., if the file containing the inputs is `commands.c`, replace all occurrences of `simple-test` with `commands`.
3. Enter `make` into the terminal to compile the program.
4. Enter `./file` where `file` is the name excluding the `.c` extension of the C file containing the input commands.

_Example:_
```
$ ls
Makefile  byos.h  byos.c  inputs.c
$ make
gcc  -c -o byos.o byos.c
gcc  -c -o simple-test.o simple-test.c
gcc -o simple-test byos.o simple-test.o
$ ./inputs
[OUTPUT DISPLAYED HERE]
```
