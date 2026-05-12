#include <iostream>
#include <vector>
#include <string>
#include "vyronix/Lexer.hpp"
#include "vyronix/Parser.hpp"
#include "vyronix/SemanticAnalyzer.hpp"
#include "vyronix/IRGenerator.hpp"
#include "vyronix/VM.hpp"
#include "vyronix/IRVerifier.hpp"

using namespace vyronix;

void run_test(const std::string& name, const std::string& source) {
    std::cout << "Running test: " << name << "..." << std::endl;
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
        std::cout << "[SUCCESS] " << name << " passed." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[FAILED] " << name << ": " << e.what() << std::endl;
    }
}

int main() {
    // 1. Type Inference
    run_test("Type Inference", R"(
        let x = 10;
        let y = "hello";
        print(x);
        print(y);
    )");

    // 2. String Interpolation
    run_test("String Interpolation", R"(
        let name = "Vyronix";
        let version = 1.0;
        print("Hello {name} v{version}!");
    )");

    // 3. Array Slicing
    run_test("Array Slicing", R"(
        let arr = [1, 2, 3, 4, 5];
        let sub = arr[1:4];
        print(sub[0]);
        print(sub[1]);
        print(sub[2]);
    )");

    // 4. String Slicing
    run_test("String Slicing", R"(
        let s = "Hello World";
        print(s[0:5]);
    )");

    return 0;
}
