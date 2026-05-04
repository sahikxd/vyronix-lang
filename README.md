# ⚡ VYRONIX Programming Language

**VYRONIX** is a high-performance, async-first programming language designed for speed, safety, and simplicity. Built on a custom C++17 engine, it features Python-like indentation, a cooperative task scheduler, and automated memory management.

![Logo](logo.png)

## ✨ Key Features

- **🚀 Async-First Architecture**: Native support for concurrency with `spawn` and `await` keywords.
- **🛡️ Memory Safety**: Automated Hybrid Mark-and-Sweep Garbage Collector + Ownership tracking.
- **🐍 Clean Syntax**: Indentation-based blocks for highly readable and maintainable code.
- **🛠️ Stack-Based VM**: Optimized IR (Intermediate Representation) execution for low-latency performance.
- **🧩 VS Code Integration**: Official extension with syntax highlighting and an integrated "Run" button.

---

## 📥 Installation

### **Windows (Recommended)**
1. **Download**: Download the latest release zip from GitHub.
2. **Extract**: Unzip the folder to a permanent location (e.g., `C:\vyronix`).
3. **Environment Variables**:
   - Open **Start Menu**, search for "Environment Variables".
   - Under **System Variables**, find `Path` and click **Edit**.
   - Click **New** and add the path to your extracted `vyronix` folder.
4. **Verify**: Open a new terminal and type:
   ```bash
   vyx version
   ```

### **Linux / macOS**
- Ensure `libcurl` is installed on your system.
- Add the directory to your shell path in `.bashrc` or `.zshrc`.

---

## 🧩 VS Code Setup (Highly Recommended)

For the best development experience, install the VYRONIX Extension:

1. Download **`vyronix-1.0.0.vsix`** from the `vscode-extension/` folder.
2. Open VS Code, go to the **Extensions** tab (`Ctrl+Shift+X`).
3. Click the `...` menu (top right) -> **Install from VSIX...**
4. Select the downloaded file.

---

## 🛠️ Usage & Commands

### **Run a Script**
Execute any `.vx` file directly:
```bash
vyx run examples/01_hello.vx
```

### **Interactive REPL**
Test code line-by-line in the interactive shell:
```bash
vyx repl
```

### **Semantic Check**
Validate your code for errors without executing it:
```bash
vyx check your_file.vx
```

---

## 📖 Language at a Glance

### **Variables & Arithmetic**
```python
let x = 10
let message = "Result: "
set x = x + 5
echo message + x  # Output: Result: 15
```

### **Functions & Logic**
```python
unit check_score(val):
    when val >= 80:
        give "Pass"
    otherwise:
        give "Fail"

let result = check_score(85)
echo "Status: " + result
```

### **Async Concurrency**
```python
unit heavy_task(id):
    echo "Task " + id + " started"
    # Logic here...
    give "Done"

let t1 = spawn heavy_task(1)
echo await t1
```

---

## 📂 Project Structure

- `vyx.exe`: The core execution engine.
- `stdlib/`: Built-in libraries for IO, Math, Net, and more.
- `examples/`: Curated scripts to help you get started.
- `vscode-extension/`: Official VSIX package for IDE support.

---
**Developed by [sahikxd](https://github.com/sahikxd)** | [Report a Bug](https://github.com/sahikxd/vyronix-lang/issues)
