#include <iostream>
#include "vyronix/Lexer.hpp"
#include "vyronix/Parser.hpp"
#include "vyronix/SemanticAnalyzer.hpp"

using namespace vyronix;

int main() {
    std::string source = 
        "let x: int = 42;\n"
        "fn add(a: int, b: int) -> int {\n"
        "  return a + b;\n"
        "}\n"
        "if (x > 40) {\n"
        "  print(add(x, 1));\n"
        "}\n"
        "// Semantic errors test:\n"
        "// let y: string = 10; // Type mismatch\n"
        "// const pi: float = 3.14;\n"
        "// pi = 3.15; // Assign to const\n";

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    std::vector<std::unique_ptr<Stmt>> statements = parser.parse();

    SemanticAnalyzer analyzer;
    std::cout << "Starting semantic analysis..." << std::endl;
    analyzer.analyze(statements);
    std::cout << "Semantic analysis completed." << std::endl;

    return 0;
}
