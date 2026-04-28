# VYRONIX Programming Language

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)

**VYRONIX** is a high-performance, systems-oriented programming language featuring a clean, indentation-based syntax and a powerful C++17-based compiler backend.

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
