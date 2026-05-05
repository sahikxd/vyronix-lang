<div align="center">

<img src="logo.png" alt="VYRONIX Logo" width="160"/>

# VYRONIX

**A modern, async-first programming language built from scratch in C++17.**

Ownership-aware · Mark-and-Sweep GC · Cooperative Async · Stack-based IR VM · Optimizer

[![Release](https://img.shields.io/github/v/release/sahikxd/vyronix-lang?color=00e5ff&label=release&style=flat-square)](https://github.com/sahikxd/vyronix-lang/releases)
[![License](https://img.shields.io/github/license/sahikxd/vyronix-lang?color=ffffff&style=flat-square)](LICENSE)
[![Stars](https://img.shields.io/github/stars/sahikxd/vyronix-lang?color=00e5ff&style=flat-square)](https://github.com/sahikxd/vyronix-lang/stargazers)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-00e5ff?style=flat-square)]()
[![Built with](https://img.shields.io/badge/built%20with-C%2B%2B17-00e5ff?style=flat-square)]()

[Install](#-installation) · [Quick Start](#-quick-start) · [Language Guide](#-language-guide) · [VS Code](#-vs-code-extension) · [Roadmap](#-roadmap)

</div>

---

## What is VYRONIX?

VYRONIX is a statically-scoped, dynamically-typed programming language with a hand-written compiler pipeline — entirely built in C++17. It features Python-like indentation syntax, a cooperative async scheduler, an ownership-tracking semantic layer, and a custom stack-based IR virtual machine with mark-and-sweep garbage collection.

It is not a wrapper around another language. Every stage — from tokenization to execution — is implemented from scratch.

```python
unit greet(name):
    let msg = "Hello, " + name + "!"
    give msg

echo greet("VYRONIX")
# Output: Hello, VYRONIX!
```

---

## Compiler Pipeline

```
Source (.vx)
    │
    ▼
 Lexer          — Indentation-aware tokenizer (INDENT/DEDENT)
    │
    ▼
 Parser         — Recursive descent → AST
    │
    ▼
 Semantic       — Type checking · Ownership (MOVED/FREED) · Scope validation
    │
    ▼
 Optimizer      — Constant folding · Dead Code Elimination (fixed-point)
    │
    ▼
 IR Generator   — AST → Stack-based IR instructions
    │
    ▼
 IR VM          — Cooperative async scheduler · Mark-and-Sweep GC
    │
    ▼
 Codegen        — IR → C++17 transpilation (thread-local globals)
```

Every phase is a standalone C++ module with explicit error propagation — no exceptions silently swallowed.

---

## Features

**Language**
- Indentation-based block structure (no braces needed)
- First-class async with `spawn` and `await`
- Ownership tracking — use-after-free caught at compile time
- `free` keyword for manual memory hints
- `when` / `otherwise` conditionals
- `unit` functions with `give` return

**Runtime**
- Cooperative task scheduler (no OS threads needed for concurrency)
- Iterative Mark-and-Sweep GC (prevents C++ stack overflow during marking)
- Division-by-zero and null-access runtime guards
- `--safe` sandboxed mode (disables IO/Net/System access)

**Tooling**
- `vyx run` — direct interpreter mode
- `vyx repl` — interactive shell with multiline support (`\` continuation)
- `vyx check` — fast semantic validation without execution
- `vyx build` — transpile to C++17 native binary *(coming soon)*
- `--debug` flag — full pipeline tracing per stage
- VS Code extension — syntax highlighting + ▶ Run button + debugger integration

**Standard Library**
- `io.vx` — input/output
- `math.vx` — arithmetic utilities
- `fs.vx` — filesystem operations
- `net.vx` — HTTP networking *(libcurl, in progress)*

---

## Installation

### Windows — Installer (Recommended)

1. Download `vyronix-0.9.4-windows-x64-setup.exe` from [Releases](https://github.com/sahikxd/vyronix-lang/releases)
2. Run the installer — it will:
   - Install VYRONIX to `C:\Program Files\VYRONIX`
   - Add `vyx` to your system PATH
   - Associate `.vx` files (double-click to run)
   - Add right-click context menu options
3. Open a new terminal and verify:

```bash
vyx version
```

### Windows — Manual

1. Download and extract the release zip to a permanent location (e.g. `C:\vyronix`)
2. Add that folder to your system `PATH` via Environment Variables
3. Place the required DLLs alongside `vyx.exe`:
   - `libgcc_s_seh-1.dll`
   - `libstdc++-6.dll`
   - `libwinpthread-1.dll`

### Linux / macOS

```bash
# Install libcurl (required for net module)
sudo apt install libcurl4        # Ubuntu/Debian
brew install curl                # macOS

# Add vyx to PATH
export PATH="$PATH:/path/to/vyronix"

# Verify
vyx version
```

---

## Quick Start

Create a file `hello.vx`:

```python
let name = "world"
echo "Hello, " + name + "!"
```

Run it:

```bash
vyx run hello.vx
```

Or just double-click `hello.vx` if you used the installer.

---

## Language Guide

### Variables

```python
let x = 42
let name = "VYRONIX"
set x = x + 8       # reassignment uses `set`
echo x               # 50
```

### Functions

```python
unit add(a, b):
    give a + b

let result = add(10, 20)
echo result    # 30
```

### Conditionals

```python
unit classify(score):
    when score >= 90:
        give "Excellent"
    when score >= 60:
        give "Pass"
    otherwise:
        give "Fail"

echo classify(75)    # Pass
```

### Loops

```python
let i = 0
loop i < 5:
    echo "i = " + i
    set i = i + 1
```

### Async Concurrency

```python
unit fetch_data(id):
    echo "Fetching " + id
    # ... work ...
    give "data-" + id

let t1 = spawn fetch_data(1)
let t2 = spawn fetch_data(2)

echo await t1
echo await t2
# Tasks run cooperatively — no blocking
```

### Ownership & Memory

```python
let buf = "important data"
echo buf

free buf           # manual release hint

# echo buf         # ← semantic error: use after free
                   # caught at compile time, not runtime
```

### Error Checking (no execution)

```bash
vyx check myfile.vx
# Semantic check passed: no errors found
```

---

## VS Code Extension

Install the official VYRONIX extension for syntax highlighting, error markers, and the integrated ▶ Run button.

**From VSIX:**
1. Download `vyronix-1.0.0.vsix` from the [`vscode-extension/`](https://github.com/sahikxd/vyronix-lang-extention) repository
2. Open VS Code → Extensions (`Ctrl+Shift+X`)
3. Click `···` → **Install from VSIX...**
4. Select the downloaded file

**Features:**
- Full syntax highlighting for `.vx` files
- ▶ Run button in the editor toolbar
- VYRONIX Debugger integration framework
- Error and warning markers

---

## CLI Reference

| Command | Description |
|---|---|
| `vyx run <file.vx>` | Execute a source file |
| `vyx repl` | Start the interactive shell |
| `vyx check <file.vx>` | Semantic validation only (no execution) |
| `vyx build <file.vx> -o` | Transpile to native C++ binary |
| `vyx version` | Show version info |
| `vyx help` | Show all commands |

**Flags:**

| Flag | Description |
|---|---|
| `--debug` | Full pipeline trace (`[LEXER]`, `[PARSER]`, `[IR]`, ...) |
| `--safe` | Sandboxed mode — disables IO, Net, System calls |

---

## Building from Source

**Requirements:**
- C++17 compiler — GCC 9+ / Clang 10+ / MSVC 2019+
- CMake 3.10+
- libcurl-dev (for net module)
- .NET 10.0+ (for VS Code extension tooling)

```bash
git clone https://github.com/sahikxd/vyronix-lang
cd vyronix-lang
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

Windows (MinGW):
```bash
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
mingw32-make
```

---

## Project Structure

```
vyronix/
├── src/
│   ├── main.cpp              # CLI, REPL, pipeline orchestration
│   ├── lexer/lexer.cpp       # Indentation-aware tokenizer
│   ├── parser/parser.cpp     # Recursive descent parser
│   ├── semantic/semantic.cpp # Type checking + ownership validation
│   ├── optimizer/optimizer.cpp # Constant folding + DCE
│   ├── ir/ir_generator.cpp   # AST → IR
│   ├── ir/ir_vm.cpp          # VM + async scheduler + GC
│   ├── codegen/codegen.cpp   # IR → C++17 transpiler
│   └── runtime/gc/gc.cpp     # Mark-and-sweep GC heap
├── stdlib/
│   ├── io.vx
│   ├── math.vx
│   ├── fs.vx
│   └── net.vx
├── examples/
└── logo.png
```

---

## Roadmap

- [x] Lexer with INDENT/DEDENT
- [x] Recursive descent parser
- [x] Semantic analysis + ownership tracking
- [x] IR generator + stack-based VM
- [x] Cooperative async scheduler
- [x] Mark-and-sweep garbage collector
- [x] Constant folding + dead code elimination
- [x] C++17 transpiler/codegen
- [x] VS Code extension
- [x] Windows installer
- [ ] `libcurl` net module integration
- [ ] LLVM native backend
- [ ] Type inference
- [ ] Generics
- [ ] Package manager
- [ ] Language server (LSP)
- [ ] vyronix.dev documentation site

---

## Contributing

Issues, bug reports, and pull requests are welcome.

```bash
git clone https://github.com/sahikxd/vyronix-lang
# make your changes
# open a pull request
```

Please include a `.vx` test case that demonstrates the bug or feature.

---

<div align="center">

Built from scratch · C++17 · No dependencies on other language runtimes

**[sahikxd](https://github.com/sahikxd)** · [Report a Bug](https://github.com/sahikxd/vyronix-lang/issues) · [VS Code Extension](https://github.com/sahikxd/vyronix-lang-extention)

</div>
