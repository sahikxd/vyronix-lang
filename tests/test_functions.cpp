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
        vm.run(code);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    std::string source = 
        "fn add(a: int, b: int) -> int {\n"
        "  return a + b;\n"
        "}\n"
        "fn multiply(x: int, y: int) -> int {\n"
        "  return x * y;\n"
        "}\n"
        "let result: int = add(5, 10);\n"
        "print(\"Result of add(5, 10):\");\n"
        "print(result);\n"
        "let prod: int = multiply(result, 2);\n"
        "print(\"Result of multiply(result, 2):\");\n"
        "print(prod);";

    std::cout << "--- Executing VYRONIX Function Test ---" << std::endl;
    runSource(source);
    std::cout << "--- Execution Completed ---" << std::endl;

    return 0;
}
