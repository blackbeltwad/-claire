# -claire

**A small systems programming language and compiler built from scratch for RISC-V.**

Éclaire is a personal programming language project focused on understanding how a programming language goes from source code all the way down to machine code.

The goal is to build the entire toolchain rather than rely on an existing compiler framework:

```text
Éclaire source
      ↓
    Lexer
      ↓
    Parser
      ↓
      AST
      ↓
Semantic analysis
      ↓
   Éclaire IR
      ↓
 Optimizations
      ↓
RISC-V codegen
      ↓
RISC-V assembly
      ↓
Assembler / linker
      ↓
Executable
      ↓
    QEMU
```

## Language

Éclaire is intended to be a low-level, statically typed language with modern language features.

Planned features include:

* Static typing with type inference
* Functions and lexical scoping
* Structs and arrays
* Pointers and pointer arithmetic
* Manual memory management
* Generics
* Compile-time evaluation
* Modules
* Useful compiler diagnostics

The language is designed to sit somewhere between the simplicity and control of C and the stronger abstractions found in modern systems languages.

## Compiler

The compiler will be written from scratch in modern C++.

The compiler will eventually contain:

* Lexer
* Parser
* AST
* Name resolution
* Type inference and type checking
* Intermediate representation
* Control-flow graphs
* SSA
* Optimization passes
* Instruction selection
* Register allocation
* RISC-V backend
* ABI and stack-frame generation
* Executable generation

The project is primarily about learning by implementing these systems rather than following a compiler tutorial or hiding the important parts behind existing abstractions.

## Target

The initial compilation target is **RISC-V**, with generated programs running under **QEMU**.

RISC-V provides a relatively clean target for understanding the relationship between:

**language → compiler → IR → instructions → ABI → machine**

## Why?

The purpose of Éclaire is not simply to create another programming language.

It is an attempt to understand what actually happens between writing:

```text
let x = 10;
```

and eventually executing machine instructions that operate on registers and memory.

The project will be developed incrementally, with the compiler, language, runtime, and generated machine code remaining understandable from the bottom up.

