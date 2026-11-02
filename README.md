# Rose

A lightweight PHP-like Syntax Analyser and AST generator written in C++17.

## Features

- **Lexer**: Tokenizes PHP-like syntax (variables, keywords, literals, operators, comments). Variable names are stripped of the leading `$` prefix.
- **Parser**: Recursive descent parser supporting expressions (arithmetic, relational, logical), control flow (`if`/`else`, `while`), function declarations, assignments, and `echo` statements.
- **AST Generation**: Visualizes the Abstract Syntax Tree with formatted hierarchy.
- **Diagnostics**: Error recovery and syntax diagnostic reporting with line and column numbers.

## Building

### Windows (PowerShell)

```powershell
.\build.ps1
```

### Linux / macOS (Make)

```bash
make
```

## Usage

### Run Test Suite

```bash
.\rosec.exe
```

### Analyze a File

```bash
.\rosec.exe examples/fibonacci.php
```

### Inline Code Analysis

```bash
.\rosec.exe -e "<?php \$x = 10; echo \$x; ?>"
```
