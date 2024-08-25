# RREX

## Regular expression interpreter / validator

This regular expression validator is made with the target to be faster than the glibc regular expression validator and with success. In 23/25 tests it scores a better result than the original glibc validator. My bytecode compiler is way faster than the one provided by glibc and my executor often. For single validation, my validator is always a better choice.

## Benchmark and test
Benchmark vs. glibc regex

![Gif of build process](build.gif)

## Todo / issues
 - Segmenation fault fix is expr ends with \\d?
 - rassert(!rrex("123", "[123]+b")); doesn't work
 - abc with abc[gg]d matches valid. Shouldn't be so

## Make

### `all:`
Runs the following tasks sequentially:
- **`one-file:`** Merges, formats, and compiles a single C file.
- **`format_all:`** Formats all `.c` and `.h` files using `clang-format`.
- **`build:`** Compiles the main program (`rrex2.c`) with optimization and static linking.
- **`run:`** Executes the compiled program (`rrex2`).

### `format_all:`
Formats all `.c` and `.h` files in the directory using `clang-format`.

### `build:`
Compiles `rrex2.c` into an executable named `rrex2` with optimization (`-O2`), all warnings enabled (`-Wall`), and extra warnings enabled (`-Wextra`). The executable is statically linked.

### `run:`
Runs the compiled program (`rrex2`).

### `test:`
Rebuilds the project by running the `build` target and then runs the program with `test` as an argument.

### `cli:`
Runs the `build` target and then executes the program in CLI mode.

### `compiler:`
Compiles `compiler.c` into an optimized executable `compiler.o` and then runs it.

### `backup:`
Creates a compressed archive (`rrex.rzip`) of all `.c`, `.h`, Makefile, and markdown files in the directory.

### `coverage:`
Generates code coverage information:
- Removes existing coverage data files.
- Compiles `rrex2.c` with profiling and test coverage flags.
- Executes the compiled coverage binary with `test` as an argument.
- Captures coverage data with `lcov` and generates an HTML report using `genhtml`.
- Opens the coverage report in Google Chrome.
- Cleans up intermediate coverage files and the binary.
