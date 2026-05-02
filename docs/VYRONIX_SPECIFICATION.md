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
