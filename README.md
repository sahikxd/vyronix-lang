# VYRONIX Programming Language

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)

**VYRONIX** is a high-performance, systems-oriented programming language that combines the developer-friendly syntax of Python with the speed and control of C++.

## 🚀 Key Features

- **Indentation-Based Syntax**: Clean, readable code without curly brace clutter.
- **Explicit Memory Control**: `alloc`, `move`, and `drop` for predictable performance.
- **Modern Concurrency**: First-class `async/await` and lightweight `spawn` threads.
- **Optimizing Compiler**: Built-in Constant Folding and Dead Code Elimination.
- **C++ Interop**: Compiles to standard C++17 for maximum portability.

## 📦 Installation

### Prerequisites
- C++17 compatible compiler (GCC 7+, Clang 5+, or MSVC 2017+)
- CMake 3.10+
- Python 3.x (for test scripts)

### Build from Source
```bash
git clone https://github.com/your-username/vyronix.git
cd vyronix
mkdir build && cd build
cmake ..
make
```

## 🛠️ Usage Guide: How to Code in VYRONIX

### 1. Creating Your First Script
Create a file with the `.vx` extension (e.g., `app.vx`).

### 2. Basic Syntax Rules
- **Indentation**: Use 4 spaces for blocks (like Python). No curly braces `{}`.
- **Variables**: Use `let` to declare and `set` to update.
- **Functions**: Defined with `unit`.

### 3. Running Your Code
You can run your code in two ways using the `vyx_latest.exe` tool:

#### **A. Interpreter Mode (Easiest)**
No compiler needed. Runs directly using the internal Virtual Machine.
```powershell
.\vyx_latest.exe interpret app.vx
```

#### **B. Compiled Mode (Fastest)**
Generates a standalone native `.exe` file. (Requires `g++` or `MSVC` installed).
```powershell
.\vyx_latest.exe build app.vx -o my_app.exe
.\my_app.exe
```

---

## 📝 Example Snippets

### Hello World & Math
```python
import "stdlib/math.vx" as math

let name = "Developer"
println("Hello, " + name + "!")

let result = math.sqrt(144.0)
println("Square root of 144 is: " + float_to_str(result))
```

### Conditions & Loops
```python
let count = 1
while count <= 5:
    when count % 2 == 0:
        println(int_to_str(count) + " is Even")
    otherwise:
        println(int_to_str(count) + " is Odd")
    set count = count + 1
```

### Working with Files
```python
let file = "data.txt"
fs_write(file, "Welcome to VYRONIX!")

when fs_exists(file):
    let content = fs_read(file)
    println("File Content: " + content)
```

---

## 📂 Project Structure

- `src/`: Core compiler implementation (Lexer, Parser, IR, etc.)
- `std/`: Standard library implementation.
- `docs/`: Technical specifications and documentation.
- `tests/`: Comprehensive test suite.
- `examples/`: Sample VYRONIX programs.

## 🗺️ Roadmap

- [ ] LLVM Backend Integration
- [ ] Language Server Protocol (LSP) support
- [ ] VYRONIX Package Manager (VPM)
- [ ] WebAssembly Support

## 🤝 Contributing

Contributions are welcome! Please see the [Contributing Guide](docs/CONTRIBUTING.md) for details.

## 📄 License

VYRONIX is licensed under the MIT License. See [LICENSE](LICENSE) for details.
