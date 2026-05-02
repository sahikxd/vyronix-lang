# VYRONIX Programming Language

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)

**VYRONIX** is a high-performance, systems-oriented programming language featuring a clean, indentation-based syntax and a powerful C++17-based compiler backend.

## Author: Mohammad Mobassir Hoque (sahik) — VYRONIX Language Creator ❤

## 🚀 Features
- **Clean Syntax**: Python-like indentation for blocks.
- **High Performance**: Compiles to native code via C++17.
- **Manual Memory Control**: Explicit `alloc`, `free`, `move`, and `copy`.
- **Concurrency**: First-class `async`/`await` and `spawn` primitives.
- **Static Typing**: Optional type annotations with robust inference.
- **Optimized**: Built-in Constant Folding and Dead Code Elimination.

## 🛠️ Installation
### Prerequisites
- C++17 compatible compiler (GCC 7+, Clang 5+, or MSVC 2017+)
- CMake 3.10+

### Building from Source
```bash
git clone https://github.com/vyronix-lang/vyronix.git
cd vyronix
mkdir build && cd build
cmake ..
make
```

## 📖 Examples
### Factorial (Recursion)
```python
unit factorial(n: int): int
    when n <= 1:
        give 1
    otherwise:
        give n * factorial(n - 1)

echo factorial(5) # Output: 120
```

### Memory Management
```python
let size = 512
alloc myBuffer size
echo "Allocated buffer"
free myBuffer
```

## 🗺️ Roadmap
- [ ] **Phase 1**: Core Lexer & Parser (Complete)
- [ ] **Phase 2**: Semantic Analysis (Complete)
- [ ] **Phase 3**: IR Generation & Optimization (Complete)
- [ ] **Phase 4**: C++ Code Generation (Complete)
- [ ] **Phase 5**: Standard Library Development (In Progress)
- [ ] **Phase 6**: Package Manager (VYM)
- [ ] **Phase 7**: LLVM Backend

## 🤝 Contributing
We welcome contributions! Please see [CONTRIBUTING.md](docs/CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## 📄 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

# 🧠 VYRONIX Compiler Internals

## 1. Compiler Pipeline
The VYRONIX compiler architecture is a classic multi-stage pipeline:
1.  **Lexer**: Converts source text into a stream of tokens, including synthetic INDENT/DEDENT tokens.
2.  **Parser**: A recursive descent parser that builds the Abstract Syntax Tree (AST).
3.  **Semantic Analyzer**: Validates types, resolves scopes, and performs symbol table management.
4.  **Optimizer**: Operates on the AST and IR to perform Constant Folding and Dead Code Elimination.
5.  **IR Generator**: Lowers the AST into a register-based Intermediate Representation.
6.  **Code Generator**: Translates IR into C++17 source code.
7.  **Backend**: Compiles C++ to native machine code using `g++`.

---

## 2. AST Design
The AST is a hierarchy of nodes representing the program structure.
- **Statements (`AST::Stmt`)**:
    - `VarDeclStmt`: `let name = expr`
    - `IfStmt`: `when cond: block otherwise: block`
    - `AllocStmt`: `alloc name size`
- **Expressions (`AST::Expr`)**:
    - `BinaryExpr`: `left op right`
    - `LiteralExpr`: Numbers, strings, booleans.
    - `CallExpr`: `func(args)`

**Memory Management**: The AST uses `std::unique_ptr` throughout. This ensures that the entire tree is owned by the `Program` node, and memory is automatically reclaimed when the program node is destroyed.

---

## 3. Semantic Analysis
The `SemanticAnalyzer` ensures the program is semantically valid before code generation.

### Symbol Table
A multi-level symbol table tracks variables across nested scopes.
```cpp
class SymbolTable {
    std::vector<std::unordered_map<std::string, Symbol>> scopes;
};
```

### Type Checking
Rules enforced:
- Binary operations must have compatible types.
- Function calls must match parameter counts and types.
- Memory operations (`move`, `copy`) must reference valid allocated symbols.

---

## 4. Intermediate Representation (IR)
The IR is a flat list of three-address instructions:
- `IR_LOAD_CONST`: Pushes a literal to the stack.
- `IR_STORE`: Stores the top of the stack into a variable.
- `IR_ALLOC`: Allocates virtual memory.
- `IR_JUMP_IF_FALSE`: Conditional branching.

The IR is designed to be easily translatable to both a VM (for interpreted execution) and C++ (for native compilation).
# 📘 VYRONIX Official Manual

## 1. Introduction
**VYRONIX** is a next-generation systems programming language designed for high performance, memory safety, and developer productivity. It features a Python-inspired indentation-based syntax that compiles directly to optimized native binaries via C++17.

### Why VYRONIX?
In the modern landscape, developers often choose between the speed of development (Python, Ruby) and the speed of execution (C, C++). VYRONIX eliminates this dichotomy. It provides a clean, readable syntax that doesn't sacrifice the low-level control required for systems engineering, game development, and high-frequency applications.

### Design Philosophy
- **Performance by Default**: No hidden runtimes or heavy garbage collection.
- **Simplicity**: A concise keyword set and clear block structure.
- **Control**: Explicit memory management where it matters.
- **Modernity**: Built-in support for asynchronous programming and modern type systems.

---

## 2. Getting Started
To begin with VYRONIX, you need the `vyx` toolchain.

### Hello World
Create a file named `hello.vx`:
```python
echo "Hello, VYRONIX!"
```

Run it:
```bash
vyx run hello.vx
```

---

## 3. Language Basics

### Variables and Types
VYRONIX uses `let` for declarations and supports optional type annotations.
```python
let count = 10          # Inferred as int
let price: float = 9.99 # Explicit type
let name = "Vyronix"    # str
```

### Functions (Units)
Functions are defined using the `unit` keyword.
```python
unit add(a: int, b: int): int
    give a + b

echo add(5, 10)
```

### Control Flow
Indentation defines blocks.
```python
when age >= 18:
    echo "Access granted"
otherwise:
    echo "Access denied"
```

---

## 4. Advanced Features

### Memory Management
VYRONIX gives you direct control over heap memory.
```python
let size = 1024
alloc myBuffer size    # Allocate 1024 bytes
# ... use buffer ...
free myBuffer          # Explicitly free memory
```

### Concurrency
Asynchronous tasks are first-class citizens.
```python
async unit processData():
    # ...
    give "Done"

let task = spawn processData()
let result = await task
```

---

## 5. Standard Library
The VYRONIX standard library provides essential modules:
- `io`: Input and output operations.
- `math`: Mathematical constants and functions.
- `fs`: File system manipulation.
- `os`: Operating system interfaces.
- `net`: Networking and socket programming.

# 🔤 VYRONIX Language Specification v1.0

## 1. Lexical Structure
VYRONIX is whitespace-sensitive. Indentation (spaces or tabs) is used to denote block levels.

### Keywords
`let`, `set`, `drop`, `echo`, `ask`, `unit`, `give`, `when`, `elif`, `otherwise`, `loop`, `while`, `class`, `self`, `init`, `new`, `yes`, `no`, `null`, `spawn`, `async`, `await`, `alloc`, `free`, `copy`, `move`, `import`, `from`, `as`, `try`, `catch`, `finally`, `raise`, `panic`.

---

## 2. EBNF Grammar
```ebnf
program        = { statement } ;
statement      = var_decl | assignment | echo_stmt | if_stmt | while_stmt 
               | func_decl | class_decl | memory_stmt | concurrency_stmt ;

block          = INDENT { statement } DEDENT ;

expression     = logical_or ;
logical_or     = logical_and { "or" logical_and } ;
logical_and    = equality { "and" equality } ;
equality       = comparison { ("==" | "!=") comparison } ;
comparison     = term { (">" | ">=" | "<" | "<=") term } ;
term           = factor { ("+" | "-") factor } ;
factor         = primary { ("*" | "/") primary } ;
primary        = NUMBER | STRING | IDENTIFIER | "(" expression ")" | call_expr | "yes" | "no" | "null" ;

var_decl       = "let" IDENTIFIER [ ":" IDENTIFIER ] [ "=" expression ] ;
assignment     = "set" IDENTIFIER "=" expression ;
echo_stmt      = "echo" expression ;

if_stmt        = "when" expression ":" block { "elif" expression ":" block } [ "otherwise" ":" block ] ;
while_stmt     = "while" expression ":" block ;

func_decl      = "unit" IDENTIFIER "(" [params] ")" [ ":" IDENTIFIER ] ":" block ;
params         = IDENTIFIER [ ":" IDENTIFIER ] { "," IDENTIFIER [ ":" IDENTIFIER ] } ;

memory_stmt    = "alloc" IDENTIFIER expression 
               | "free" IDENTIFIER 
               | "copy" IDENTIFIER IDENTIFIER 
               | "move" IDENTIFIER IDENTIFIER ;

concurrency_stmt = "spawn" call_expr | "await" IDENTIFIER ;
```

---

## 3. Type System
### Built-in Types
- `int`: 64-bit signed integer.
- `float`: 64-bit floating point.
- `str`: UTF-8 encoded string.
- `bool`: Boolean (`yes` or `no`).
- `void`: Empty return type.
- `ptr`: Raw memory address.
- `byte`: 8-bit unsigned integer.

---

## 4. Memory Model
VYRONIX implements a deterministic memory model:
1.  **Stack Allocation**: Local variables are automatically reclaimed.
2.  **Manual Heap**: `alloc` and `free` provide explicit control.
3.  **Ownership Semantics**: `move` transfers resource ownership, preventing double-free and use-after-free bugs at the compiler level.

---

## 5. Error Handling
VYRONIX uses a structured exception model:
```python
try:
    # Risky operation
    let file = open("data.txt")
catch error:
    echo "Failed to open file"
finally:
    echo "Cleanup complete"
```
# VYRONIX Language Ecosystem Specification

## 📘 1. OFFICIAL LANGUAGE DOCUMENTATION

**What is VYRONIX?**
VYRONIX is a high-performance, systems-oriented programming language designed for the modern era. It combines the expressive syntax of Python with the raw power and memory control of C++. VYRONIX is built to bridge the gap between high-level application development and low-level systems programming.

**Why it exists?**
Modern development often forces a trade-off: use a high-level language (Python/JS) for speed of development but sacrifice performance, or use a low-level language (C++/Rust) for performance but deal with complex syntax and manual management. VYRONIX eliminates this trade-off by providing an indentation-based syntax that compiles directly to highly optimized native code.

**Design Philosophy**
- **Readability First**: Code should be easy to read and maintain, using significant whitespace.
- **Explicit over Implicit**: Memory operations (alloc, move, drop) are explicit to ensure the developer always knows what the hardware is doing.
- **Safety with Control**: Provide safe defaults while allowing "system" mode for direct pointer manipulation.
- **Zero-Cost Abstractions**: High-level features (classes, async) should not impose a runtime penalty over manual implementation.

**Goals**
- **Performance**: Match or exceed C++ performance through optimized IR and C++ code generation.
- **Simplicity**: A minimal but powerful keyword set that is easy to learn.
- **Control**: Fine-grained memory management and system-level access.
- **Concurrency**: First-class support for asynchronous programming and lightweight threads (spawning).

---

## ⚙️ 2. FULL COMPILER ARCHITECTURE

The VYRONIX compiler (VYX) follows a multi-stage pipeline designed for maximum optimization and portability.

**The Pipeline:**
1.  **Source (.vx)**: Raw VYRONIX source code.
2.  **Lexer**: Tokenizes the source, handling INDENT/DEDENT tokens for block scoping and positional metadata.
3.  **Parser**: Constructs an Abstract Syntax Tree (AST) using recursive descent, enforcing the formal grammar.
4.  **Semantic Analyzer**: Performs type checking, scope resolution, and validates symbol tables. It ensures that variables are declared before use and types are compatible.
5.  **IR Generator**: Lowers the high-level AST into a linear, three-address Intermediate Representation (IR).
6.  **Optimizer**: Performs passes on the IR, including Constant Folding and Dead Code Elimination (DCE).
7.  **Code Generator (C++)**: Translates optimized IR into standard C++17 code.
8.  **Backend (g++)**: Invokes the host C++ compiler to produce a high-performance native binary.

---

## 🔤 3. FULL LANGUAGE SPECIFICATION

**EBNF Grammar (Simplified)**
```ebnf
program      = { statement } ;
statement    = var_decl | assignment | echo_stmt | if_stmt | loop_stmt | func_decl | class_decl ;
block        = INDENT { statement } DEDENT ;
expr         = term { ("+" | "-") term } ;
term         = factor { ("*" | "/") factor } ;
factor       = NUMBER | STRING | IDENTIFIER | "(" expr ")" | call_expr ;
var_decl     = "let" IDENTIFIER "=" expr ;
func_decl    = "unit" IDENTIFIER "(" [params] ")" ":" block ;
```

**Keywords**
- **Variables**: `let` (declare), `set` (assign), `drop` (manual free).
- **Control**: `when`, `elif`, `otherwise`, `loop`, `while`.
- **Functions/Classes**: `unit`, `give` (return), `class`, `self`, `init`, `new`.
- **Memory**: `alloc`, `free`, `copy`, `move`.
- **Concurrency**: `spawn`, `async`, `await`.
- **System**: `import`, `from`, `as`, `try`, `catch`, `panic`.

**Data Types**
- **Primitives**: `int`, `float`, `str`, `bool`, `byte`.
- **Collections**: `list`, `map`.
- **System**: `ptr` (raw pointer), `void`.

**Memory Model**
VYRONIX uses a **Hybrid Ownership Model**. By default, objects are scope-bound. For dynamic allocation:
- `alloc`: Allocates memory on the heap.
- `move`: Transfers ownership of a resource, invalidating the previous handle.
- `drop`: Explicitly destroys an object and frees memory.

---

## 🌳 4. AST DESIGN SPEC

The AST is designed using a class hierarchy with `std::unique_ptr` for strict ownership and memory safety during compilation.

- **Base Node**: `AST::Node` (Virtual base)
- **Expression Nodes**:
    - `NumberExpr`, `StringExpr`, `BinaryExpr` (LHS, Op, RHS), `CallExpr`.
- **Statement Nodes**:
    - `VarDeclStmt`, `BlockStmt` (vector of unique_ptr<Stmt>), `IfStmt`, `LoopStmt`.
- **Declaration Nodes**:
    - `FunctionDecl` (Name, Params, Body), `ClassDecl`.

**Ownership**: Each parent node in the AST owns its children via `unique_ptr`. This ensures that when the root node is destroyed, the entire tree is safely deallocated without leaks.

---

## 🧠 5. SEMANTIC ANALYZER DESIGN

The Semantic Analyzer is the "brain" of the compiler, ensuring the code makes logical sense.

- **Symbol Table**: A stack of hash maps representing nested scopes. Each entry stores type information and visibility.
- **Type Checking**: Enforces strict typing (e.g., cannot add a `str` to an `int`).
- **Variable Resolution**: Checks that variables are declared in the current or parent scope.
- **Validation**: Ensures `give` statements match function return types and `self` is only used inside classes.
- **Error Reporting**: Generates "Error at Line X, Col Y: Type Mismatch".

---

## ⚡ 6. IR (INTERMEDIATE REPRESENTATION)

The VYRONIX IR is a flat, register-based instruction set designed for easy optimization.

**Instruction Set (V-IR):**
- `LOAD_CONST <val>, <reg>`: Load literal into virtual register.
- `LOAD_VAR <name>, <reg>`: Load variable value into register.
- `STORE_VAR <reg>, <name>`: Save register value to variable.
- `ADD/SUB/MUL/DIV <r1>, <r2>, <target>`: Arithmetic operations.
- `JMP <label>`, `JMP_IF_FALSE <reg>, <label>`: Control flow.
- `CALL <name>, <args_count>`, `RET <reg>`: Function calls.
- `ALLOC <size>, <reg>`, `FREE <reg>`: Memory operations.

**Lowering**: The IR Generator traverses the AST and emits these instructions, converting complex expressions into linear sequences.

---

## 🧪 7. EXAMPLE PROGRAMS

**Hello World**
```python
echo "Hello, VYRONIX!"
```

**Recursion (Factorial)**
```python
unit factorial(n):
    when n <= 1:
        give 1
    otherwise:
        give n * factorial(n - 1)

let result = factorial(5)
echo result # Output: 120
```

**Concurrency**
```python
async unit fetchData():
    # Simulate network
    give "Data Received"

let task = spawn fetchData()
let data = await task
echo data
```

---

## 🖥️ 8. CLI TOOL DESIGN (VYX)

The `vyx` tool is the primary interface for developers.

- `vyx run <file>`: Compiles and executes code in-memory (JIT-like behavior).
- `vyx build <file>`: Generates a standalone native binary.
- `vyx compile <file>`: Generates the intermediate C++ source code.
- `vyx fmt <file>`: Automatically formats code according to the style guide.
- `vyx test`: Runs all tests in the `/tests` directory.
- `vyx doc`: Generates documentation from source comments.
- `vyx repl`: Starts an interactive VYRONIX shell.

---

## 📦 9. STANDARD LIBRARY DESIGN

- `io`: `print()`, `scan()`, `read_file()`, `write_file()`.
- `math`: `sin()`, `cos()`, `sqrt()`, `random()`.
- `fs`: `mkdir()`, `rm()`, `exists()`.
- `net`: `http_get()`, `tcp_server()`.
- `async`: `sleep()`, `wait_all()`.
- `string`: `split()`, `join()`, `replace()`, `trim()`.
- `os`: `get_env()`, `exit()`, `args()`.

---

## 🌐 10. OFFICIAL WEBSITE DESIGN

- **Homepage**: Hero section with a code snippet, feature highlights, and download buttons.
- **Documentation**: Sidebar-based navigation for tutorials, language reference, and standard library.
- **Playground**: An online editor that compiles VYRONIX to WASM and runs it in the browser.
- **Community**: Links to GitHub, Discord, and package registry.

---

## 📁 11. GITHUB REPOSITORY STRUCTURE

```text
/vyronix
  /src
    /lexer       # Lexical analysis
    /parser      # AST and grammar
    /semantic    # Type checking
    /ir          # IR generation and VM
    /optimizer   # Optimization passes
    /codegen     # C++ code generation
    /runtime     # Garbage collection and memory
  /std           # Standard library (.vx and .cpp)
  /examples      # Sample code
  /docs          # Documentation
  /tools         # VYX CLI source
  CMakeLists.txt
  README.md
```

---

## 📄 12. PROFESSIONAL README.md

(A full README content is included in the root directory's README.md file)

---

## 🚀 13. FUTURE ROADMAP

1.  **Phase 8**: LLVM Backend integration for direct machine code generation.
2.  **Phase 9**: VYRONIX Package Manager (`vpm`).
3.  **Phase 10**: LSP (Language Server Protocol) for VS Code support.
4.  **Phase 11**: WebAssembly (WASM) target for browser execution.
5.  **Phase 12**: Advanced JIT compilation for the VM mode.
