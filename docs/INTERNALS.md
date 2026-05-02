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
