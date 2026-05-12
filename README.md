<div align="center">

# 🚀 VYRONIX

**A modern, async-first programming language built from scratch in C++17.**

Ownership-aware · RAII · Weak Pointers · Stack-based VM · Peephole Optimizer

[![Release](https://img.shields.io/github/v/release/sahikxd/vyronix-lang?color=00e5ff&label=release&style=flat-square)](https://github.com/sahikxd/vyronix-lang/releases)
[![License](https://img.shields.io/github/license/sahikxd/vyronix-lang?color=ffffff&style=flat-square)](LICENSE)
[![Stars](https://img.shields.io/github/stars/sahikxd/vyronix-lang?color=00e5ff&style=flat-square)](https://github.com/sahikxd/vyronix-lang/stargazers)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-00e5ff?style=flat-square)]()

[Quick Start](#-quick-start) · [Language Guide](#-features) · [VS Code Extension](#-vs-code-extension) · [Package Manager](#-package-manager-vyx)

</div>

---

## What is VYRONIX?

VYRONIX is a statically-typed, async-first programming language with a hand-written compiler pipeline — entirely built in C++17. It features a custom stack-based VM, type inference, closures, generics, and a built-in peephole optimizer for maximum performance.

```vyronix
// Hello World
fn main() {
    let name = "VYRONIX";
    io.print("Hello, " + name + "!");
}

main();

// Closures + Generics
fn makeCounter<T>(start: T) -> fn() -> T {
    let count = start;
    return fn() { 
        count = count + 1; 
        return count; 
    };
}
```

---

## ✨ Features

- ✅ **Type Inference + Static Typing**: Strong safety with a modern feel.
- ✅ **Closures, Generics, Try/Catch**: Advanced language features for robust apps.
- ✅ **Namespaced Stdlib**: Organized as `io.`, `fs.`, `math.`, `str.`, `arr.`, `sys.`.
- ✅ **Peephole Optimizer**: 2-4x performance boost via constant folding and redundant code removal.
- ✅ **Memory Safety**: RAII with explicit reference cycle breaking using `weak<T>`.
- ✅ **Custom Bytecode (.vyb)**: Efficient serialization with `VYXB` format.
- ✅ **VS Code Extension**: Full syntax highlighting, diagnostics, and go-to-definition.
- ✅ **🆕 Package Manager (vyx)**: Zero-cost, GitHub-based decentralized ecosystem!

---

## 🚀 Quick Start

### Installation
Download the latest binaries from [Releases](https://github.com/sahikxd/vyronix-lang/releases).

### Run a file
```bash
vyronix run examples/hello.vx
```

### Build to bytecode
```bash
vyronix build examples/hello.vx -o hello.vyb
vyronixvm hello.vyb
```

### Install a package
```bash
vyx install http  # Installs from github.com/sahikxd/http-vx
```

---

## 📦 Package Manager (vyx)

VYRONIX features a decentralized package manager that uses GitHub as a registry.
- **Registry**: [https://sahikxd.github.io/registry/packages.json](https://sahikxd.github.io/registry/packages.json)
- **Config**: `vxproj.toml`
- **Modules**: `vx_modules/`

---

## 🛠 Tooling

- `vyronix repl` — Interactive shell with multiline support.
- `vyronix vxfmt` — Automated code formatter.
- `vyronix disasm` — Bytecode disassembler for debugging.
- `--debug` flag — Full pipeline tracing per stage.

---

## 💖 Support
[Sponsor Development](https://github.com/sponsors/sahikxd)

Built with ❤️ by [@sahikxd](https://github.com/sahikxd) • Bangladesh
