# Mock Shell

## Approach
This mock shell supports three commands - Echo, Forx and List. In my implementation, each command is handled by a separate function.

## Function Overview


## Running the Program

1. Ensure `Makefile` is in the same directory as `byos.c`.
2. `Makefile` has currently been configured to handle inputs specified in a file called `simple-test.c`. Please substitute all occurrences of `simple-test` in the `Makefile` with the name excluding the `.c` extension of the C file containing the input commands  (e.g., if the file containing the inputs is `commands.c`, replace all occurrences of `simple-test` with `commands`.
3. Enter `make` into the terminal to compile the program.
4. Enter `./file` where `file` is the name excluding the `.c` extension of the C file containing the input commands.
