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
    std::string cmd = "curl -L -s -o \"" + outPath + "\" \"" + url + "\"";
    return std::system(cmd.c_str()) == 0;
}

bool extractZip(const std::string& zipPath, const std::string& outDir) {
    fs::create_directories(outDir);
    std::string cmd = "tar -xf \"" + zipPath + "\" -C \"" + outDir + "\" --strip-components=1";
    return std::system(cmd.c_str()) == 0;
}

void installPackage(const std::string& pkg) {
    std::string repo = pkg;
    std::string alias = pkg;
    std::string tag = "main";

    if (pkg.find('/') == std::string::npos) {
        // Look up in registry (mock for now)
        if (pkg == "http") repo = "sahikxd/http-vx";
        else if (pkg == "json") repo = "sahikxd/json-vx";
        else {
            std::cerr << "Error: Unknown alias '" << pkg << "'. Use user/repo format.\n";
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
