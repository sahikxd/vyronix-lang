# 📦 vyx Package Manager

`vyx` is the official, decentralized package manager for the VYRONIX programming language. It allows you to manage dependencies without any server costs by leveraging GitHub as a static registry.

## Commands

### `vyx init`
Initializes a new VYRONIX project by creating a `vxproj.toml` file.

### `vyx install <package>`
Installs a package from the registry or directly from GitHub.
- **Example (Registry Alias)**: `vyx install http`
- **Example (GitHub Repo)**: `vyx install sahikxd/http-vx`

### `vyx update`
Updates all dependencies in `vx_modules/` based on the latest versions in the registry.

## How it Works

1. **Registry Lookup**: `vyx` fetches the `packages.json` file from the [VYRONIX Registry](https://sahikxd.github.io/registry/packages.json).
2. **Resolution**: It resolves the alias (e.g., `http`) to a GitHub repository URL.
3. **Download**: The source code is downloaded as a ZIP archive directly from GitHub's infrastructure.
4. **Extraction**: The archive is extracted into the `vx_modules/` directory of your project.
5. **Importing**: You can now use the package in your code:
   ```vyronix
   import http;
   
   fn main() {
       let res = http.get("https://api.vyronix.org");
       io.print(res.body);
   }
   ```

## Creating a Package

To make your library installable via `vyx`:
1. Create a `vxproj.toml` in your repository root.
2. Structure your code with `src/lib.vx` as the entry point.
3. Tag your releases (e.g., `v1.0.0`).
4. (Optional) Submit a PR to the [Registry repository](https://github.com/sahikxd/registry) to add an alias.
