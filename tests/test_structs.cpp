#include <iostream>
#include "vyronix/Lexer.hpp"
#include "vyronix/Parser.hpp"
#include "vyronix/SemanticAnalyzer.hpp"
#include "vyronix/IRGenerator.hpp"
#include "vyronix/VM.hpp"

using namespace vyronix;

void runSource(const std::string& source) {
    try {
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();
        Parser parser(tokens);
        std::vector<std::unique_ptr<Stmt>> statements = parser.parse();
        SemanticAnalyzer analyzer;
        analyzer.analyze(statements);
        IRGenerator irGen;
        std::vector<Instruction> code = irGen.generate(statements);
        VirtualMachine vm;
        try {
            vm.run(code);
        } catch (const std::exception& e) {
            std::cerr << "Runtime Error: " << e.what() << std::endl;
            vm.dumpState(code);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    std::string source = 
        "struct Point {\n"
        "  x: int;\n"
        "  y: int;\n"
        "}\n"
        "let p: Point = Point { x: 10, y: 20 };\n"
        "print(\"Point p.x:\");\n"
        "print(p.x);\n"
        "print(\"Point p.y:\");\n"
        "print(p.y);\n"
        "p.x = 100;\n"
        "print(\"Updated p.x:\");\n"
        "print(p.x);";

    std::cout << "--- Executing VYRONIX Struct Test ---" << std::endl;
    runSource(source);
    std::cout << "--- Execution Completed ---" << std::endl;

    return 0;
}
