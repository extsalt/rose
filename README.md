# The Rose Compiler: Architecture, Specification, and Engineering Handbook

> **A Comprehensive Guide and Technical Manual for the Rose PHP-like Syntax Analyser and AST Generator**

---

## Table of Contents

- [Front Matter](#front-matter)
  - [Project Charter](#project-charter)
  - [Design Philosophy](#design-philosophy)
- [Part I: Architectural Blueprint](#part-i-architectural-blueprint)
  - [Chapter 1: System Overview & Component Pipeline](#chapter-1-system-overview--component-pipeline)
  - [Chapter 2: Formal Grammar Specification (EBNF)](#chapter-2-formal-grammar-specification-ebnf)
- [Part II: Front-End Compiler Internals](#part-ii-front-end-compiler-internals)
  - [Chapter 3: Lexical Analysis (Scanner & Tokenizer)](#chapter-3-lexical-analysis-scanner--tokenizer)
  - [Chapter 4: Recursive Descent Syntax Analysis](#chapter-4-recursive-descent-syntax-analysis)
  - [Chapter 5: Abstract Syntax Tree (AST) Domain Model](#chapter-5-abstract-syntax-tree-ast-domain-model)
  - [Chapter 6: Error Diagnostics & Panic-Mode Recovery](#chapter-6-error-diagnostics--panic-mode-recovery)
- [Part III: Operational Engineering](#part-iii-operational-engineering)
  - [Chapter 7: Build Toolchains & Platform Instructions](#chapter-7-build-toolchains--platform-instructions)
  - [Chapter 8: Verification Suites & Execution Handbook](#chapter-8-verification-suites--execution-handbook)
- [Part IV: Comprehensive Future Roadmap](#part-iv-comprehensive-future-roadmap)
  - [Chapter 9: Phased Engineering Plan](#chapter-9-phased-engineering-plan)
  - [Chapter 10: Technical Milestones & Milestone Matrix](#chapter-10-technical-milestones--milestone-matrix)
- [Appendix: Codebase Directory Index](#appendix-codebase-directory-index)

---

## Front Matter

### Project Charter

- **Mission:** Provide a clean, robust, and extensible front-end syntax analyzer and AST generator for a PHP-like procedural language.
- **Implementation Standard:** Pure C++17 with zero third-party dependencies, leveraging standard containers, RAII pointer semantics, and modern type systems.
- **Core Scope:**
  - Lexical tokenization of source code including tags, identifiers, keywords, literals, and comments.
  - Variable sigil stripping (storing variables cleanly as canonical identifiers without the leading `$`).
  - Precedence-climbing recursive descent expression parsing.
  - Hierarchical statement parsing and AST construction.
  - Resilient error recovery supporting multi-error diagnostics.

### Design Philosophy

- **Zero External Dependencies:** Built entirely against the C++17 Standard Library (`<string>`, `<vector>`, `<memory>`, `<iostream>`).
- **Memory Safety via RAII:** Strict ownership modeling through `std::unique_ptr` across AST nodes to eliminate manual memory management and leak vectors.
- **Resilient Compilation:** The parser avoids aborting on the first syntax error, employing panic-mode synchronization to report multiple diagnostics per pass.
- **Clean Semantic Representations:** Tokens and AST nodes separate lexical syntax markers (like `$` sigils and quotes) from semantic identifiers and literal values.

---

## Part I: Architectural Blueprint

### Chapter 1: System Overview & Component Pipeline

- **Compiler Pipeline Stages:**
  1. **Source Ingestion:** Reads source files or raw strings via standard I/O streams.
  2. **Lexical Scanning (`Lexer`):** Converts character streams into structured, position-tagged tokens (`Token`).
  3. **Syntax Analysis (`Parser`):** Consumes tokens via recursive descent and builds hierarchical syntax structures.
  4. **AST Construction (`ASTNode`):** Assembles an immutable tree of strongly typed statements (`StmtAST`) and expressions (`ExprAST`).
  5. **Tree Serialization (`dump`):** Formats and outputs the AST to stream targets with configurable visual indentation.
  6. **Diagnostic Reporting:** Aggregates error records (`ParserError`) with precise line and column coordinate tags.

```
       +------------------------------------+
       |         Source Code (PHP)          |
       +------------------------------------+
                         |
                         v
       +------------------------------------+
       |       Lexer (src/Lexer.cpp)        |
       |  - Tokenization                    |
       |  - Strips '$' from variable names  |
       |  - Unescapes string literals       |
       +------------------------------------+
                         |
                         v  std::vector<Token>
       +------------------------------------+
       |      Parser (src/Parser.cpp)       |
       |  - Recursive descent parsing       |
       |  - Operator precedence climbing    |
       |  - Panic-mode error recovery       |
       +------------------------------------+
                         |
           +-------------+-------------+
           |                           |
           v (Success)                 v (Diagnostics)
 +--------------------+      +--------------------+
 | ProgramAST / Nodes |      |   Parser Errors    |
 | - StmtAST          |      |  - Line & Column   |
 | - ExprAST          |      |  - Error Message   |
 +--------------------+      +--------------------+
           |
           v
 +--------------------+
 |  stdout / Visual   |
 +--------------------+
```

---

### Chapter 2: Formal Grammar Specification (EBNF)

- **Top-Level Program Grammar:**
  ```ebnf
  Program        ::= [ PhpOpenTag ] { Declaration | Statement } [ PhpCloseTag ] EOF ;
  PhpOpenTag     ::= "<?php" ;
  PhpCloseTag    ::= "?>" ;
  ```

- **Declarations & Statements:**
  ```ebnf
  Declaration    ::= FunctionDecl | Statement ;
  FunctionDecl   ::= "function" Identifier "(" [ ParameterList ] ")" BlockStmt ;
  ParameterList  ::= Variable { "," Variable } ;

  Statement      ::= BlockStmt
                   | IfStmt
                   | WhileStmt
                   | ReturnStmt
                   | EchoStmt
                   | ExprStmt ;

  BlockStmt      ::= "{" { Statement } "}" ;
  IfStmt         ::= "if" "(" Expression ")" Statement [ "else" Statement ] ;
  WhileStmt      ::= "while" "(" Expression ")" Statement ;
  ReturnStmt     ::= "return" [ Expression ] ";" ;
  EchoStmt       ::= "echo" Expression { "," Expression } ";" ;
  ExprStmt       ::= Expression ";" ;
  ```

- **Expressions & Operator Precedence (Lowest to Highest):**
  ```ebnf
  Expression     ::= Assignment ;
  Assignment     ::= Variable "=" Assignment | LogicalOr ;
  LogicalOr      ::= LogicalAnd { "||" LogicalAnd } ;
  LogicalAnd     ::= Equality { "&&" Equality } ;
  Equality       ::= Relational { ( "==" | "!=" ) Relational } ;
  Relational     ::= Additive { ( "<" | "<=" | ">" | ">=" ) Additive } ;
  Additive       ::= Multiplicative { ( "+" | "-" ) Multiplicative } ;
  Multiplicative ::= Unary { ( "*" | "/" | "%" ) Unary } ;
  Unary          ::= ( "!" | "-" ) Unary | Primary ;
  Primary        ::= IntLiteral
                   | FloatLiteral
                   | StringLiteral
                   | BoolLiteral
                   | NullLiteral
                   | Variable
                   | CallExpr
                   | "(" Expression ")" ;
  CallExpr       ::= Identifier "(" [ ArgumentList ] ")" ;
  ArgumentList   ::= Expression { "," Expression } ;
  ```

- **Lexical Tokens:**
  ```ebnf
  Variable       ::= "$" [a-zA-Z_] [a-zA-Z0-9_]* ;  (* '$' stripped in token representation *)
  Identifier     ::= [a-zA-Z_] [a-zA-Z0-9_]* ;
  IntLiteral     ::= [0-9]+ ;
  FloatLiteral   ::= [0-9]+ "." [0-9]+ ;
  StringLiteral  ::= '"' { EscapeSeq | [^"\] } '"' | "'" { EscapeSeq | [^'\] } "'" ;
  ```

---

## Part II: Front-End Compiler Internals

### Chapter 3: Lexical Analysis (Scanner & Tokenizer)

- **Source Location Tracking:**
  - Maintains `line` (1-indexed) and `column` (1-indexed) coordinates.
  - Records `startColumn` at the beginning of each token scan for pinpoint diagnostic attribution.

- **Sigil Handling for Variables:**
  - Detection: Triggered when character is `$` followed by an alphabetic character or underscore.
  - Stripping Policy: The lexer slices `source.substr(start + 1, current - (start + 1))` so the stored token text contains purely the variable's name (e.g. `total`, `count`), omitting `$`.

- **Case-Insensitive Keyword Matching:**
  - The scanner converts keyword text candidates to lowercase during lookup.
  - Supported Keywords: `function`, `if`, `else`, `while`, `return`, `echo`.

- **Literal Processing & Escape Sequences:**
  - Strings: Strips bounding single or double quotes; decodes `\n`, `\t`, `\r`, `\\`, `\"`, `\'`, and `\$`.
  - Numerics: Disambiguates integers and floating-point values via decimal point inspection.
  - Booleans & Null: Mapped to first-class literal tokens (`TokenType::BoolLiteral`, `TokenType::NullLiteral`).

- **Whitespace & Comment Handling:**
  - Discards whitespace (spaces, tabs, carriage returns, newlines) while adjusting line/column tracking.
  - Supports C++ single-line comments (`//`), shell-style single-line comments (`#`), and C-style block comments (`/* ... */`).

---

### Chapter 4: Recursive Descent Syntax Analysis

- **Architecture of the Parser:**
  - Implements a deterministic recursive descent algorithm operating over a contiguous token vector.
  - Maintains `current` token index with `peek()`, `peekNext()`, `previous()`, and `isAtEnd()` utilities.

- **Precedence Hierarchy Table:**
  | Precedence | Level Name | Operators | Associativity |
  | :--- | :--- | :--- | :--- |
  | **1 (Lowest)** | Assignment | `=` | Right |
  | **2** | Logical OR | `\|\|` | Left |
  | **3** | Logical AND | `&&` | Left |
  | **4** | Equality | `==`, `!=` | Left |
  | **5** | Relational | `<`, `<=`, `>`, `>=` | Left |
  | **6** | Additive | `+`, `-` | Left |
  | **7** | Multiplicative | `*`, `/`, `%` | Left |
  | **8 (Highest)** | Unary | `!`, `-` | Right |

- **Lookahead & Disambiguation:**
  - Assignments vs Expressions: Inspects `check(TokenType::Variable) && peekNext().type == TokenType::Equal` to branch into `parseAssignment()`.
  - Function Calls vs Identifiers: Differentiates identifier calls via `TokenType::Identifier` followed by `TokenType::LParen`.

---

### Chapter 5: Abstract Syntax Tree (AST) Domain Model

- **Base Class Hierarchy:**
  - `ASTNode`: Abstract root providing polymorphic `virtual void dump(int indent, std::ostream& out) const = 0`.
  - `ExprAST`: Base class for value-yielding syntax constructs.
  - `StmtAST`: Base class for executable statements.

- **Expression Node Catalog:**
  - `IntExprAST` / `FloatExprAST`: Constant numeric values stored as `int64_t` and `double`.
  - `StringExprAST`: String literals stored without delimiters.
  - `BoolExprAST` / `NullExprAST`: Boolean primitives (`true`/`false`) and `null`.
  - `VariableExprAST`: Variable reference. Canonical identifier stored in `name` without `$`.
  - `BinaryExprAST`: Holds `op` string, `unique_ptr<ExprAST> lhs`, and `unique_ptr<ExprAST> rhs`.
  - `UnaryExprAST`: Holds `op` string and `unique_ptr<ExprAST> operand`.
  - `AssignExprAST`: Holds target `varName` (without `$`) and assigned value `unique_ptr<ExprAST> value`.
  - `CallExprAST`: Holds `callee` name and `std::vector<std::unique_ptr<ExprAST>> args`.

- **Statement Node Catalog:**
  - `ExprStmtAST`: Wraps an isolated expression statement ending in `;`.
  - `EchoStmtAST`: Holds a comma-separated list of output expressions.
  - `ReturnStmtAST`: Holds an optional return expression (or `nullptr` for void return).
  - `BlockStmtAST`: Holds an ordered sequence of inner statements within `{ ... }`.
  - `IfStmtAST`: Holds `condition`, `thenBranch`, and optional `elseBranch`.
  - `WhileStmtAST`: Holds loop `condition` and `body` statement.
  - `FunctionDeclAST`: Holds function `name`, parameter list `params` (strings without `$`), and body `BlockStmtAST`.
  - `ProgramAST`: Top-level root holding the complete sequence of declarations and statements.

---

### Chapter 6: Error Diagnostics & Panic-Mode Recovery

- **Diagnostic Record Schema:**
  - `SourceLocation`: Line and column of the offending token.
  - `message`: Clear, human-readable description of expected syntax vs actual encounter.
  - `tokenText`: Literal string slice at fault for contextual clarity.

- **Panic-Mode Synchronization:**
  - When encountering unrecoverable expression or statement errors, the parser logs the diagnostic and invokes `synchronize()`.
  - Recovery Strategy: Discards tokens until encountering a statement boundary (semicolon `;`) or one of the synchronization anchor keywords (`function`, `if`, `while`, `return`, `echo`).
  - Benefit: Prevents cascading false errors and ensures downstream functions/statements are analyzed.

---

## Part III: Operational Engineering

### Chapter 7: Build Toolchains & Platform Instructions

- **Prerequisites:**
  - Modern C++17 compatible compiler (`g++` >= 8, `clang++` >= 7, or MSVC 2019+).
  - PowerShell (on Windows) or GNU Make (on Linux/macOS).

- **Windows Build (PowerShell):**
  ```powershell
  # Compile executable to rosec.exe
  .\build.ps1

  # Clean build artifacts
  .\build.ps1 -Clean
  ```

- **Linux / macOS Build (Makefile):**
  ```bash
  # Compile executable to rosec
  make

  # Clean build artifacts
  make clean
  ```

---

### Chapter 8: Verification Suites & Execution Handbook

- **Execution Modes:**
  1. **Built-in Demonstration Suite:**
     ```bash
     .\rosec.exe
     ```
     Executes four built-in test suites covering variable assignments, control flow structures, function declarations, and error handling diagnostics.

  2. **File Analysis Mode:**
     ```bash
     .\rosec.exe examples\fibonacci.php
     .\rosec.exe examples\hello.php
     ```
     Loads an external source file, executes full lexical and syntax analysis, and prints the formatted AST.

  3. **Inline Expression / Code Analysis Mode:**
     ```bash
     .\rosec.exe -e "<?php \$x = 42; echo \$x; ?>"
     ```
     Parses inline code snippets and prints the full token stream alongside the generated AST.

---

## Part IV: Comprehensive Future Roadmap

### Chapter 9: Phased Engineering Plan

- **Phase 1: Language Syntax Extensions**
  - **Associative & Indexed Arrays:** Add bracket syntax `[1, 2, 3]` and key-value pairs `['key' => 'value']`.
  - **Looping Constructs:** Add `for ($i = 0; $i < 10; $i++)` and `foreach ($items as $item)` syntax.
  - **Branching Controls:** Add `switch` / `case` and modern `match` expressions.
  - **Type Annotations:** Support optional scalar type hints on function parameters and return types (e.g. `function add(int $a, int $b): int`).

- **Phase 2: Semantic Analysis & Symbol Management**
  - **Scoped Symbol Tables:** Construct lexical scope stacks tracking variable declarations, function signatures, and scoping boundaries.
  - **Static Type Checking:** Validate operator operand compatibility, detecting type mismatch errors prior to execution.
  - **Variable Initialization Analysis:** Detect references to undefined or uninitialized variables.

- **Phase 3: Intermediate Representation (IR)**
  - **Control Flow Graph (CFG):** Transform AST nodes into basic blocks linked by directed edges.
  - **Three-Address Code (3AC) / SSA Form:** Generate linear intermediate representations in Static Single Assignment form for optimization passes.

- **Phase 4: Optimization Engine**
  - **Constant Folding & Propagation:** Evaluate compile-time constant arithmetic expressions.
  - **Dead Code Elimination:** Strip unreachable blocks and unused variable assignments.
  - **Common Subexpression Elimination (CSE):** Cache repeated deterministic calculations.

- **Phase 5: Execution Runtime & Code Generation**
  - **Bytecode Virtual Machine:** Design a stack-based or register-based bytecode instruction set and fast dispatch interpreter.
  - **JIT / LLVM Backend:** Bridge the IR to LLVM infrastructure for high-performance machine code compilation.

---

### Chapter 10: Technical Milestones & Milestone Matrix

| Milestone | Target Objective | Key Deliverables | Risk Level |
| :--- | :--- | :--- | :--- |
| **M1: Core Stabilization** | Solidify C++17 front-end | Sigil-free variables, error sync, clean AST dumps | Low (Completed) |
| **M2: Extended Constructs** | Arrays, `for`, `foreach` | Array literal tokens, subscript operator, loop AST nodes | Medium |
| **M3: Semantic Pipeline** | Scopes & Symbol Table | Scope stack, variable lifetime checks, semantic validator | Medium |
| **M4: IR Generation** | SSA Intermediate Form | Basic block builder, CFG visualizer, 3AC generation | High |
| **M5: Runtime Execution** | Virtual Machine Execution | Bytecode assembler, stack interpreter, benchmark suite | High |

---

## Appendix: Codebase Directory Index

- **Header Declarations (`include/`):**
  - [`include/Token.h`](include/Token.h): Defines `TokenType` enumeration, `SourceLocation`, and `Token` structure.
  - [`include/Lexer.h`](include/Lexer.h): Scanner class interface and character traversal methods.
  - [`include/AST.h`](include/AST.h): Complete object-oriented AST node hierarchy (`ExprAST`, `StmtAST`, `ProgramAST`).
  - [`include/Parser.h`](include/Parser.h): Recursive descent parser class interface and diagnostic container.

- **Source Implementations (`src/`):**
  - [`src/Token.cpp`](src/Token.cpp): Token-to-string mapping dictionary.
  - [`src/Lexer.cpp`](src/Lexer.cpp): State-machine tokenizer, comment skipper, string unescaper, and variable sigil stripper.
  - [`src/AST.cpp`](src/AST.cpp): Hierarchical AST node serializers implementing indented visual dumping.
  - [`src/Parser.cpp`](src/Parser.cpp): Recursive descent implementation, precedence climbers, and panic-mode error recovery.

- **Entry Point & Tooling:**
  - [`main.cpp`](main.cpp): CLI argument dispatcher, built-in test harness, and diagnostics printer.
  - [`build.ps1`](build.ps1): PowerShell automation build script for Windows targets.
  - [`Makefile`](Makefile): Standard GNU Makefile for POSIX targets.
  - [`examples/`](examples/): Example programs ([`fibonacci.php`](examples/fibonacci.php), [`hello.php`](examples/hello.php)).
