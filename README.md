<div align="center">

# 🚀 VYRONIX

**A modern, async-first programming language built from scratch in C++17.**

Ownership-aware · RAII · Weak Pointers · Stack-based VM · Peephole Optimizer

[![Release](https://img.shields.io/github/v/release/sahikxd/vyronix-lang?color=00e5ff&label=release&style=flat-square)](https://github.com/sahikxd/vyronix-lang/releases)
[![License](https://img.shields.io/github/license/sahikxd/vyronix-lang?color=ffffff&style=flat-square)](LICENSE)
[![Stars](https://img.shields.io/github/stars/sahikxd/vyronix-lang?color=00e5ff&style=flat-square)](https://github.com/sahikxd/vyronix-lang/stargazers)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-00e5ff?style=flat-square)]()

[Quick Start](#-quick-start) · [Language Guide](#-language-guide) · [Standard Library](#-standard-library) · [Package Manager](#-package-manager-vyx)

</div>

---

## 💡 What is VYRONIX?

VYRONIX is a statically-typed, high-performance systems programming language. It is designed to be fast, memory-safe, and easy to use. Every part of the toolchain — from the lexer to the custom virtual machine — is hand-written in C++17.

### Why VYRONIX?
- **Speed**: Built-in peephole optimizer for efficient bytecode.
- **Safety**: RAII and `weak<T>` pointers prevent memory leaks and cycles.
- **Simplicity**: Clean C-style syntax with powerful type inference.
- **Ecosystem**: A built-in, zero-cost package manager (`vyx`).

---

## 📖 Language Guide & Examples

### 1. Variables & Types
VYRONIX uses `let` for mutable variables and `const` for immutable ones. Types can be explicitly stated or inferred.

```vyronix
let x = 10;                // Inferred as i64
const pi: f64 = 3.14159;   // Explicit type
let name = "VYRONIX";      // String
```

### 2. Functions & Closures
Functions are first-class citizens. You can pass them around, return them, and capture variables from the surrounding scope.

```vyronix
// Standard function
fn add(a: i64, b: i64) -> i64 {
    return a + b;
}

// Higher-order function with closure
fn multiplier(factor: i64) -> fn(i64) -> i64 {
    return fn(n: i64) { 
        return n * factor; 
    };
}

let double = multiplier(2);
io.print(double(5)); // Output: 10
```

### 3. Data Structures (Structs & Arrays)
Define custom data models and manage collections easily.

```vyronix
struct Player {
    name: str,
    score: i64
}

let p1 = Player { name: "Sahik", score: 100 };
let scores = [10, 20, 30]; // Array literal

io.print(p1.name + " scored " + str.from(p1.score));
```

### 4. Generics
Write reusable code that works with any data type.

```vyronix
fn swap<T>(pair: (T, T)) -> (T, T) {
    return (pair.1, pair.0);
}

let swapped = swap<i64>((1, 2)); // Returns (2, 1)
```

### 5. Error Handling
Robust error management using `try-catch` blocks.

```vyronix
try {
    if (1 > 0) {
        throw "System error occurred!";
    }
} catch (err) {
    io.print("Caught: " + err);
}
```

---

## 🚀 Quick Start

### 1. Installation
Download the latest binaries for your OS from the [Releases](https://github.com/sahikxd/vyronix-lang/releases) page.

### 2. Running Code
Create a file `hello.vx`:
```vyronix
fn main() {
    io.print("Hello, VYRONIX!");
}
main();
```
Run it instantly:
```bash
vyronix run hello.vx
```

### 3. Building & Disassembling
Compile to optimized binary format:
```bash
vyronix build hello.vx -o hello.vyb
vyronix disasm hello.vyb  # Peek at the optimized bytecode!
```

---

## 📦 Package Manager (vyx)

`vyx` allows you to install libraries directly from GitHub.

```bash
vyx init             # Initialize vxproj.toml
vyx install http      # Install http-vx from registry
```

---

## 🛠 Standard Library (Namespaced)
- `io`: `print`, `input`
- `math`: `sqrt`, `pow`, `sin`, `cos`
- `str`: `len`, `split`, `upper`, `lower`
- `fs`: `read`, `write`, `exists`
- `arr`: `push`, `pop`, `sort`

---

## 💖 Support
[Sponsor Development](https://github.com/sponsors/sahikxd)

Built with ❤️ by [@sahikxd](https://github.com/sahikxd) • Bangladesh
