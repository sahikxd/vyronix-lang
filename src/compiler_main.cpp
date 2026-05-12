#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <memory>

#include "vyronix/Lexer.hpp"
#include "vyronix/Parser.hpp"
#include "vyronix/SemanticAnalyzer.hpp"
#include "vyronix/IRGenerator.hpp"
#include "vyronix/IROptimizer.hpp"
#include "vyronix/IRVerifier.hpp"
#include "vyronix/Serialization.hpp"
#include "vyronix/VM.hpp"
#include "vyronix/Error.hpp"

using namespace vyronix;

void printUsage() {
    std::cout << "VYRONIX Compiler (vyronixc) — built " << __DATE__ << " " << __TIME__ << "\n";
    std::cout << "Usage: vyronixc <input.vx> [-o <output.vyb>] [--debug]\n";
}

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) throw std::runtime_error("Could not open file: " + path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    // 0. Initialize Standard Library
    NativeRegistry::getInstance().setupStdLib();

    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = "out.vyb";
    bool debug = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            output_file = argv[++i];
        } else if (arg == "--debug") {
            debug = true;
        }
    }

    try {
        std::string source = readFile(input_file);
        
        Lexer lexer(source, input_file);
        auto tokens = lexer.tokenize();

        Parser parser(tokens, input_file);
        auto statements = parser.parse();
        if (debug) std::cout << "[DEBUG] Parsing complete.\n";

        SemanticAnalyzer analyzer;
        if (debug) std::cout << "[DEBUG] Starting semantic analysis...\n";
        analyzer.analyze(statements, input_file);
        if (debug) std::cout << "[DEBUG] Semantic analysis complete.\n";

        IRGenerator irgen;
        if (debug) std::cout << "[DEBUG] Starting IR generation...\n";
        auto code = irgen.generate(statements);

        IROptimizer optimizer;
        if (debug) std::cout << "[DEBUG] Starting IR optimization...\n";
        code = optimizer.optimize(code);

        if (debug) {
            std::cout << "[DEBUG] IR generation & optimization complete (" << code.size() << " instructions).\n";
            std::cout << "--- DISASSEMBLY ---\n";
            VirtualMachine::disassemble(code);
        }

        if (debug) std::cout << "[DEBUG] Starting IR verification...\n";
        IRVerifier::verify(code, input_file);
        if (debug) std::cout << "[DEBUG] IR verification complete.\n";

        if (debug) std::cout << "[DEBUG] Starting serialization to " << output_file << "...\n";
        if (!Serializer::serialize(code, output_file)) {
            std::cerr << "Error: Failed to write bytecode to " << output_file << "\n";
            return 1;
        }

        if (debug) {
            std::cout << "Bytecode saved to " << output_file << "\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Compilation error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
