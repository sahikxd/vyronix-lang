# VYRONIX VS Code Extension

This extension provides comprehensive support for the **VYRONIX** programming language.

## Features

- **Syntax Highlighting**: Beautiful color coding for `.vx` files.
- **Snippets**: Quick shortcuts for functions, loops, structs, and more.
- **Run Command**: Execute your VYRONIX code directly from VS Code.
- **Language Configuration**: Auto-closing brackets and smart indentation.

## Requirements

To run VYRONIX files, you must have the VYRONIX compiler (`vyronixc.exe`) and VM (`vyronixvm.exe`) built and accessible.

## Extension Settings

This extension contributes the following settings:

* `vyronix.compilerPath`: Absolute path to the VYRONIX compiler executable.
* `vyronix.vmPath`: Absolute path to the VYRONIX VM executable.

## Usage

1. Open a `.vx` file.
2. Press `Ctrl+Shift+P` and type `Vyronix: Run File`.
3. The output will appear in the **VYRONIX Output** channel.

---
**Ecosystem Version: 1.0**
