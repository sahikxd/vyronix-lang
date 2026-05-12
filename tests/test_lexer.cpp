#include <iostream>
#include <iomanip>
#include "vyronix/Lexer.hpp"

using namespace vyronix;

void printToken(const Token& token) {
    std::cout << std::left << std::setw(15) << static_cast<int>(token.type)
              << std::setw(15) << token.lexeme
              << "Line: " << token.line << " Col: " << token.column << std::endl;
}

int main() {
    std::string source = 
        "let x: int = 42;\n"
        "const pi: float = 3.14159;\n"
        "fn add(a: int, b: int) -> int {\n"
        "  return a + b;\n"
        "}\n"
        "print(\"Hello, Vyronix!\");";

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    for (const auto& token : tokens) {
        printToken(token);
    }

    return 0;
}
