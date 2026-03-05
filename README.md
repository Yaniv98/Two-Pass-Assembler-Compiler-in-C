# Custom Two-Pass Assembler

> **Author:** Yaniv Schwarzschild  
> **Date of submission:** 7.6.2023
> **Course:** Systems Programming Laboratory (20465), Open university (aka OpenU).
> **Instructor:** Nadav Beno

A custom two-pass assembler written in ANSI C, developed as part of the Systems Programming Laboratory course.
This program translates a fictional assembly language into base64 machine code, simulating the complete assembly process including macro deployment, memory allocation, and symbol resolution.

## The Assembly Language Architecture
This assembler processes a custom assembly language designed specifically for a fictional 12-bit CPU architecture.

**Hardware & System Specifications:**
* **Instruction Set:** 16 distinct operations (e.g., `mov`, `cmp`, `add`, `jmp`, `jsr`, `stop`).
* **Word Size:** 12 bits per memory cell.
* **Memory Capacity:** 1024 memory words (RAM).
* **Registers:** 8 general-purpose registers (`@r0` through `@r7`).
* **Data Representation:** Operates using 12-bit two's complement for numeric values (ranging from -2048 to 2047).
* **Addressing Modes:** Supports Immediate (e.g., `-5`), Direct (e.g., `LABEL`), and Direct-Register (e.g., `@r3`) addressing methods.

## Features
* **Macro Preprocessor:** Automatically detects and expands macros (`mcro` ... `endmcro`) before the first pass, generating an intermediate `.am` file.
* **Two-Pass Architecture:**
  * **First Pass:** Scans the code, validates syntax, builds the Symbol Table (labels, external, and entry points), and calculates the memory footprint for instructions and data.
  * **Second Pass:** Resolves symbolic addresses, completes the binary encoding for missing operands, and generates the final output files.
* **Robust Error Handling:** Detects and reports syntax errors, memory overflows, undefined labels, and invalid addressing methods, specifying the exact file and line number.
* **Custom Machine Code Output:** The final binary output is converted into a unique Base64 representation.

## File Structure
The project is divided into modular C files, each responsible for a specific component of the compilation process:
* `main.c` - Program entry point and flow control.
* `preProcessor.c` & `macros.c` - Handling the macro expansion phase.
* `firstPass.c` & `secondPass.c` - Core logic for the assembler's two passes.
* `lineManager.c` & `stringUtil.c` - Parsing, tokenizing, and validating individual lines of assembly code.
* `memory.c` - Managing the Instruction Counter (IC), Data Counter (DC), and symbol tables.
* `operations.c` & `operands.c` - Instruction decoding and operand validation.
* `labels.c` - Symbol table management.
* `errors.c` - Unified error reporting system.

## Build Instructions
The project includes a `makefile` for easy compilation. To build the assembler, simply open your terminal in the project directory and run:
```bash
make
```
This will compile all .c files and generate an executable named assembler.

## How to run
Run the assembler by passing the names of your assembly source files (without the .as extension) as command-line arguments. You can process multiple files in a single run:
```bash
./assembler file1 file2
```
## Input
* <filename>.as : The source assembly file.

## Output files
If the assembly code is valid and contains no errors, the program will generate the following files:
<filename>.am: The post-macro expansion source code.
<filename>.ob: The object file containing the translated base64 machine code.
<filename>.ent: (If applicable) A list of all .entry symbols and their resolved addresses.
<filename>.ext: (If applicable) A list of all .extern symbols and the addresses where they are used.

## Cleanup
To remove all compiled object files and generated outputs (.am, .ob, .ent, .ext), run:
```bash
make clean
```
## Tests & Samples
The repository includes a comprehensive set of test cases to demonstrate the assembler's capabilities and robustness.

### Tests Directory
The `/Tests` directory contains various assembly source files (`.as`) used for integration testing:
* **Valid Inputs:** `valid1.as`, `valid2.as`, and `valid3.as` demonstrate successful assembly of different instruction sets and directives.
* **Error Handling:** `error1.as` and `error2.as` contain intentional syntax and logical errors to test the program's error detection and reporting mechanisms.
* **Complex Example:** `instructionsExample.as` provides a broad use-case of the fictional language's features.

### Sample Directory (Output Example)
To illustrate the complete compilation flow, the `/Sample` directory contains both the source and all generated output files for a single representative case (`instructionsExample`):
* `instructionsExample.as`: The original source code.
* `instructionsExample.am`: The source after macro expansion.
* `instructionsExample.ob`: The final object code in Base64 format.
* `instructionsExample.ent` & `instructionsExample.ext`: The resolved entry and external symbol tables.

## How to Run Tests
To run the assembler on the provided test cases, first build the project using `make`, then execute:

# Example: Running a valid test case
```bash
./assembler Tests/valid1
```
# Example: Running a test case with errors
```bash
./assembler Tests/error1
```
