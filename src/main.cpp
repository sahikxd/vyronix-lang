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
#include "vyronix/VM.hpp"
#include "vyronix/Error.hpp"
#include "vyronix/IRVerifier.hpp"
#include "vyronix/Serialization.hpp"

using namespace vyronix;

enum ExitCode {
    EX_OK            = 0,
    EX_USAGE_ERROR   = 1, // bad args or unknown command
    EX_COMPILE_ERROR = 2, // syntax, semantic, or IR verification
    EX_RUNTIME_ERROR = 3, // VM crash or execution error
    EX_SYSTEM_ERROR  = 4  // file not found, IO errors, etc.
};

void printUsage() {
    std::cout << "VYRONIX CLI Tool — built " << __DATE__ << " " << __TIME__ << "\n";
    std::cout << "Usage: vyronix <command> <file.vx> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  run   <file.vx>  : Compile and run the VYRONIX program\n";
    std::cout << "  disasm <file.vyb> : Disassemble the VYRONIX bytecode file\n";
    std::cout << "  repl             : Start interactive REPL mode\n";
    std::cout << "  vxfmt <file.vx>  : Format the VYRONIX source file\n";
    std::cout << "  build <file.vx>  : Compile the program and verify IR generation\n";
    std::cout << "  check <file.vx>  : Perform syntax and semantic checks\n";
    std::cout << "  help             : Show this help message\n";
    std::cout << "  version          : Show version information (--version, -v)\n\n";
    std::cout << "Options:\n";
    std::cout << "  --debug          : Dump AST, IR, and provide stack traces\n";
    std::cout << "  --verbose        : Show human-readable analysis (symbol tables)\n";
    std::cout << "  --crash-dump     : Output VM state on runtime errors\n";
}

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw FileReadError("Could not open file: " + path, path, 0, 0);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    // 0. Initialize Standard Library
    NativeRegistry::getInstance().setupStdLib();

    // 1. Basic Argument Check
    if (argc < 2) {
        printUsage();
        return EX_USAGE_ERROR;
    }

    // 2. Command Validation & Global Flags
    std::string command = argv[1];
    if (command == "help" || command == "--help" || command == "-h") {
        printUsage();
        return EX_OK;
    }
    
    if (command == "version" || command == "--version" || command == "-v") {
        std::cout << "VYRONIX 1.0.0 (Reliability-Hardened Engineering Beta)\n";
        std::cout << "Built: " << __DATE__ << " " << __TIME__ << "\n";
        return EX_OK;
    }

    if (command == "repl") {
        std::cout << "VYRONIX REPL (Reliability-Hardened Beta)\n";
        std::cout << "Type 'exit' to quit.\n";
        VirtualMachine vm;
        SemanticAnalyzer analyzer;
        std::string line;
        while (true) {
            std::cout << ">> ";
            if (!std::getline(std::cin, line) || line == "exit") break;
            if (line.empty()) continue;
            
            try {
                Lexer lexer(line, "repl");
                auto tokens = lexer.tokenize();
                Parser parser(tokens, "repl");
                auto statements = parser.parse();
                
                analyzer.analyze(statements, "repl");
                
                IRGenerator irgen;
                auto code = irgen.generate(statements);
                vm.run(code);
                
                // Clear state for next line if needed, but VM/Analyzer should persist
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
        return EX_OK;
    }

    // 2.5 Multi-command Check (vxfmt, etc.)
    if (command == "vxfmt") {
        if (argc < 3) {
            std::cerr << "Error: Missing file argument for 'vxfmt'\n";
            return EX_USAGE_ERROR;
        }
        std::string filename = argv[2];
        std::cout << "VYRONIX Formatter (vxfmt) - Experimental\n";
        std::string source = readFile(filename);
        Lexer lexer(source, filename);
        auto tokens = lexer.tokenize();
        
        // Very basic formatter: just print tokens with spacing
        for (const auto& token : tokens) {
            if (token.type == TokenType::EOF_TOKEN) break;
            std::cout << token.lexeme << " ";
            if (token.type == TokenType::SEMICOLON || token.type == TokenType::LBRACE || token.type == TokenType::RBRACE) {
                std::cout << "\n";
            }
        }
        return EX_OK;
    }

    if (command == "disasm") {
        if (argc < 3) {
            std::cerr << "Error: Missing .vyb file argument for 'disasm'\n";
            return EX_USAGE_ERROR;
        }
        std::string vyb_file = argv[2];
        try {
            auto code = Serializer::deserialize(vyb_file);
            std::cout << "--- DISASSEMBLY OF " << vyb_file << " ---\n";
            VirtualMachine::disassemble(code);
        } catch (const std::exception& e) {
            std::cerr << "Disassembly error: " << e.what() << std::endl;
            return EX_SYSTEM_ERROR;
        }
        return EX_OK;
    }

    if (command != "run" && command != "build" && command != "check") {
        std::cerr << "Error: Unknown command '" << command << "'\n";
        printUsage();
        return EX_USAGE_ERROR;
    }

    // 3. File Argument Check
    if (argc < 3) {
        std::cerr << "Error: Missing file argument for command '" << command << "'\n";
        printUsage();
        return EX_USAGE_ERROR;
    }

    std::string filename = argv[2];
    
    // 4. File Extension Warning
    if (filename.size() < 4 || filename.substr(filename.size() - 3) != ".vx") {
        std::cout << "[Warning] File '" << filename << "' does not have a .vx extension.\n";
    }

    // 5. Options Parsing
    bool debug = false;
    bool verbose = false;
    bool crash_dump = false;
    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug") {
            debug = true;
        } else if (arg == "--verbose") {
            verbose = true;
        } else if (arg == "--crash-dump") {
            crash_dump = true;
        } else {
            std::cerr << "[Warning] Unknown option '" << arg << "' ignored.\n";
        }
    }

    // 6. Flag Effectiveness Validation
    if (crash_dump && command != "run") {
        std::cerr << "[Warning] --crash-dump is only effective with 'run' command.\n";
    }
    if (verbose && command == "run") {
        std::cerr << "[Warning] --verbose has no effect with 'run'. Use 'check' or 'build'.\n";
    }

    // 7. File Reading
    std::string source;
    try {
        source = readFile(filename);
    } catch (const VyronixError& e) {
        e.report("");
        return EX_SYSTEM_ERROR;
    }

    try {
        // 8. Compiler Pipeline
        // --- LEXING ---
        Lexer lexer(source, filename);
        auto tokens = lexer.tokenize();

        // --- PARSING ---
        Parser parser(tokens, filename);
        auto statements = parser.parse();

        if (debug) {
            std::cout << "[DEBUG] AST Generated (" << statements.size() << " top-level statements)\n";
        }

        // --- SEMANTIC ANALYSIS ---
        SemanticAnalyzer analyzer;
        try {
            analyzer.analyze(statements, filename);
        } catch (const SemanticError& e) {
            e.report(source);
            return 1;
        }

        if (command == "check") {
            std::cout << "Semantic check passed." << std::endl;
            return 0;
        }

        // --- IR GENERATION ---
        IRGenerator irgen;
        std::vector<Instruction> code;
        try {
            code = irgen.generate(statements);
        } catch (const std::exception& e) {
            std::cerr << "IR Generation failed: " << e.what() << std::endl;
            return 1;
        }

        // --- IR OPTIMIZATION ---
        IROptimizer optimizer;
        code = optimizer.optimize(code);

        // --- IR VERIFICATION ---
        try {
            IRVerifier::verify(code, filename);
        } catch (const RuntimeError& e) {
            if (debug) {
                std::cerr << "[DEBUG] IR Verification failed: " << e.what() << std::endl;
                VirtualMachine vm_dummy;
                vm_dummy.disassemble(code);
            } else {
                std::cerr << "IR Verification failed: " << e.what() << std::endl;
            }
            // Skip execution if IR is invalid
            return 1;
        }

        if (debug) {
            std::cout << "--- DISASSEMBLY ---" << std::endl;
            VirtualMachine::disassemble(code);
        }

        // --- EXECUTION ---
        VirtualMachine vm;
        try {
            vm.run(code);
        } catch (const VyronixError& e) {
            e.report(source);
            if (debug) vm.dumpState(code);
            return EX_RUNTIME_ERROR;
        } catch (const std::exception& e) {
            std::cerr << "Fatal error: " << e.what() << std::endl;
            return EX_RUNTIME_ERROR;
        }
    } catch (const VyronixError& e) {
        e.report(source);
        return EX_COMPILE_ERROR;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EX_SYSTEM_ERROR;
    }

    return EX_OK;
}
