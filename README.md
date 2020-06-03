# C0 Compiler

## Description
Homework for Compiler Course

2019-2020 Spring Semester

School of Computer Science and Technology, NWPU

## New Features Added
C0 syntax except for negative integer

## Implementation Steps
1) Lexical Analysis
  - recognize tokens and lexical error
  - files
    - add scanner.h  scanner.cpp
    - add error.h  error.cpp
    - add main.cpp
  
2) Syntax Analysis
  - recognize syntax structure and syntax error --> generate AST
  - files
    - add parser.h  parser.cpp
    - fix error.h   error.cpp
    - fix main.cpp
    
3) Semantic Analysis
  - typecheck --> generate symbol table
  - build symbol table while parsing
  - check with symbol table in both 2)parsing and 3)traversing ast
  - files
    - add symbolTable.h  symbolTable.cpp
    - add analyzer.h  analyzer.cpp
    - fix parser.h  parser.cpp 
    - fix main.cpp
    
4) Intermediate Code Generation
  - generate three-address code
  - build quadruples while parsing
  - files
    - add tacgen.h  tacgen.cpp
    - fix parser.h  parser.cpp
    - fix error.h  error.cpp
    - fix main.cpp

## Sources
### STAGE1
Hand-written LL(1) analysis

Construct symbol table

Build AST

Intermediate code generation

### STAGE2
Static syntax check
