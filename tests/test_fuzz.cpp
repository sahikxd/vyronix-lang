#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include "vyronix/Lexer.hpp"
#include "vyronix/Parser.hpp"
#include "vyronix/SemanticAnalyzer.hpp"
#include "vyronix/IRGenerator.hpp"
#include "vyronix/VM.hpp"
#include "vyronix/IRVerifier.hpp"

using namespace vyronix;

std::string generate_random_string(size_t length) {
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 \n\t(){}[],;+-*/%=!<>|&\"";
    std::string res;
    for (size_t i = 0; i < length; ++i) {
        res += chars[rand() % chars.length()];
    }
    return res;
}

void fuzz_lexer(int iterations) {
    std::cout << "Fuzzing Lexer..." << std::endl;
    for (int i = 0; i < iterations; ++i) {
        std::string source = generate_random_string(rand() % 500);
        try {
            Lexer lexer(source);
            lexer.tokenize();
        } catch (...) {}
    }
}

void fuzz_pipeline(int iterations) {
    std::cout << "Fuzzing Full Pipeline..." << std::endl;
    for (int i = 0; i < iterations; ++i) {
        std::string source = generate_random_string(rand() % 200);
        try {
            Lexer lexer(source);
            auto tokens = lexer.tokenize();
            Parser parser(tokens);
            auto stmts = parser.parse();
            
            SemanticAnalyzer analyzer;
            analyzer.analyze(stmts);
            
            IRGenerator generator;
            auto code = generator.generate(stmts);
            
            IRVerifier::verify(code);
            
            VirtualMachine vm;
            vm.run(code);
        } catch (...) {}
    }
}

int main() {
    srand(time(0));
    fuzz_lexer(1000);
    fuzz_pipeline(500);
    std::cout << "Fuzz testing completed without crashes." << std::endl;
    return 0;
}
