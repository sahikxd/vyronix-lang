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
