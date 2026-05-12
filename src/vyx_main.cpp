#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

void printUsage() {
    std::cout << "vyx - VYRONIX Package Manager\n";
    std::cout << "Usage: vyx <command> [args]\n";
    std::cout << "Commands:\n";
    std::cout << "  init              Initialize a new project\n";
    std::cout << "  install <pkg>     Install a package (e.g., user/repo or alias)\n";
    std::cout << "  update            Update dependencies\n";
}

bool downloadFile(const std::string& url, const std::string& outPath) {
    // -f flag tells curl to fail silently on server errors (like 404)
    std::string cmd = "curl -L -f -s -o \"" + outPath + "\" \"" + url + "\"";
    int result = std::system(cmd.c_str());
    if (result != 0) {
        return false;
    }
    // Check if file is empty or too small (404 pages saved as files)
    if (!fs::exists(outPath) || fs::file_size(outPath) < 10) {
        return false;
    }
    return true;
}

bool extractZip(const std::string& zipPath, const std::string& outDir) {
    fs::create_directories(outDir);
    // Use powershell's Expand-Archive for better ZIP handling on Windows
    std::string cmd = "powershell -Command \"Expand-Archive -Path '" + zipPath + "' -DestinationPath '" + outDir + "' -Force\"";
    int result = std::system(cmd.c_str());
    if (result != 0) return false;

    // Move files from subfolder if necessary (GitHub zips contain a root folder)
    for (const auto& entry : fs::directory_iterator(outDir)) {
        if (entry.is_directory()) {
            std::string subDir = entry.path().string();
            // Move contents of subDir to outDir
            for (const auto& subEntry : fs::directory_iterator(subDir)) {
                fs::rename(subEntry.path(), fs::path(outDir) / subEntry.path().filename());
            }
            fs::remove_all(subDir);
            break;
        }
    }
    return true;
}

std::string resolveAlias(const std::string& alias) {
    std::string registryUrl = "https://sahikxd.github.io/registry/packages.json";
    std::string tempJson = "registry_tmp.json";
    
    if (!downloadFile(registryUrl, tempJson)) {
        return "";
    }

    std::ifstream file(tempJson);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    fs::remove(tempJson);

    // Simple manual JSON parser for "alias": { "repository": "..." } or "repo": "..."
    size_t aliasPos = content.find("\"" + alias + "\"");
    if (aliasPos == std::string::npos) return "";

    size_t repoKeyPos = content.find("\"repository\"", aliasPos);
    if (repoKeyPos == std::string::npos) {
        repoKeyPos = content.find("\"repo\"", aliasPos);
    }
    
    if (repoKeyPos == std::string::npos) return "";

    size_t colonPos = content.find(":", repoKeyPos);
    size_t startQuote = content.find("\"", colonPos);
    size_t endQuote = content.find("\"", startQuote + 1);

    if (startQuote != std::string::npos && endQuote != std::string::npos) {
        std::string repo = content.substr(startQuote + 1, endQuote - startQuote - 1);
        // Remove "https://github.com/" or "github.com/" prefix if present
        if (repo.find("https://github.com/") == 0) {
            repo = repo.substr(19);
        } else if (repo.find("github.com/") == 0) {
            repo = repo.substr(11);
        }
        return repo;
    }

    return "";
}

void installPackage(const std::string& pkg) {
    std::string repo = pkg;
    std::string alias = pkg;
    std::string tag = "main";

    if (pkg.find('/') == std::string::npos) {
        std::cout << "Looking up '" << pkg << "' in registry...\n";
        repo = resolveAlias(pkg);
        if (repo.empty()) {
            std::cerr << "Error: Package '" << pkg << "' not found in registry. Use user/repo format.\n";
            return;
        }
        alias = pkg;
    } else {
        size_t last_slash = pkg.find_last_of('/');
        alias = pkg.substr(last_slash + 1);
    }

    std::string url = "https://github.com/" + repo + "/archive/refs/heads/" + tag + ".zip";
    std::string tempZip = "temp_pkg.zip";
    std::string targetDir = "vx_modules/" + alias;

    std::cout << "Installing " << pkg << " from " << url << "...\n";

    if (!downloadFile(url, tempZip)) {
        std::cerr << "Error: Failed to download " << pkg << "\n";
        return;
    }

    if (!extractZip(tempZip, targetDir)) {
        std::cerr << "Error: Failed to extract " << pkg << "\n";
        fs::remove(tempZip);
        return;
    }

    fs::remove(tempZip);
    std::cout << "✓ Successfully installed " << alias << " to " << targetDir << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "init") {
        std::ofstream file("vxproj.toml");
        file << "[project]\nname = \"my-project\"\nversion = \"0.1.0\"\n\n[dependencies]\n";
        std::cout << "Initialized vxproj.toml\n";
    } else if (command == "install") {
        if (argc < 3) {
            std::cerr << "Error: Package name required.\n";
            return 1;
        }
        installPackage(argv[2]);
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        return 1;
    }

    return 0;
}
