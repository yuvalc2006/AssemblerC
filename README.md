# AssemblerC

## Introduction

AssemblerC is a C-based assembler developed as a student project. It translates assembly language code into machine code, enabling the execution of low-level instructions on a computer. This project serves as an educational tool to understand the workings of assemblers and the translation process from human-readable code to machine-executable code.

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Dependencies](#dependencies)
- [Configuration](#configuration)
- [Examples](#examples)
- [Troubleshooting](#troubleshooting)
- [Contributors](#contributors)
- [License](#license)

## Features

- **Two-Pass Assembly Process**: Implements a two-pass algorithm to handle forward references and generate accurate machine code.
- **Symbol Table Management**: Efficiently manages symbols and labels within the assembly code.
- **Error Detection**: Identifies and reports syntax errors in the assembly source code.

## Installation

To build AssemblerC, ensure you have a C compiler installed (e.g., GCC). Then, clone the repository and compile the source code using the provided makefile:

```bash
git clone https://github.com/yuvalc2006/AssemblerC.git
cd AssemblerC
make
```

This will generate the executable `assembler` in the project directory.

## Usage

To assemble an assembly language file, run the following command:

```bash
./assembler <sourcefile.asm>
```

Replace `<sourcefile.asm>` with the path to your assembly source file. The assembler will process the input file and generate the corresponding files:
  1. .am file that contains the input file ofther the preassembler stage
  2. .object file that contains the machine code.
  3. .externals file that contains information about all of the places in the machine code where and external label was refrenced.
  4. .entries file that contains information about every label that's an entry point

## Dependencies

AssemblerC is implemented in C and requires a standard C compiler for building the project.

## Configuration

No special configuration is required. Ensure that the assembly source files are correctly formatted and follow the expected syntax.

## Examples

To assemble a file named `program.asm`:

```bash
./assembler program.asm
```

This command will process `program.asm` and produce the machine code output.

## Troubleshooting

- **Compilation Errors**: Ensure that all source files are present and that you have the necessary permissions to compile and execute the code.
- **Assembly Errors**: Verify that your assembly source files are correctly formatted and adhere to the expected syntax.

## Contributors

- [Yuval Cohen](https://github.com/yuvalc2006)

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

*Note: This project is intended for educational purposes.* 
