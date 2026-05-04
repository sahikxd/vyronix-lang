# VYRONIX Programming Language

**VYRONIX** is a high-performance, async-first programming language built with C++17. It features Python-like indentation, a cooperative task scheduler, and a hybrid mark-and-sweep garbage collector.

![Logo](logo.png)

## 🚀 Key Features

- **Async/Spawn Native**: Built-in keywords for concurrency (`spawn`, `await`).
- **Memory Safety**: Automated GC and ownership tracking (Move/Free semantics).
- **Fast Execution**: Stack-based IR VM with optimized opcodes.
- **Easy Syntax**: Indentation-based blocks, no semi-colons required.
- **Portable**: Run directly via `vyx.exe` or compile to native C++.

## 📥 Installation

1. **Download**: Clone or download this repository.
2. **Environment**: Add the folder containing `vyx.exe` to your System PATH.
3. **Dependencies**: (Windows) Ensure the included DLLs are in the same folder as `vyx.exe`.

## � Usage

### Run a Script
```bash
vyx run examples/01_hello.vx
```

### Interactive REPL
```bash
vyx repl
```

### Check for Errors
```bash
vyx check your_file.vx
```

## 🧩 VS Code Support
Download and install our official extension for Syntax Highlighting and a one-click Run button:
[VYRONIX VS Code Extension](https://github.com/sahikxd/vyronix-lang-extention)

## � Quick Example
```python
# Simple greeting function
unit greet(name):
    echo "Hello, " + name

let user = ask "Enter name: "
greet(user)
```

---
Developed by **sahikxd** | [Report Issues](https://github.com/sahikxd/vyronix-lang/issues)
