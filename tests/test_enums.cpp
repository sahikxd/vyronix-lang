#include "vyronix/Lexer.hpp"
#include "vyronix/Parser.hpp"
#include "vyronix/SemanticAnalyzer.hpp"
#include "vyronix/IRGenerator.hpp"
#include "vyronix/VM.hpp"
#include <iostream>
#include <cassert>

using namespace vyronix;

void run_code(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto stmts = parser.parse();
    
    SemanticAnalyzer analyzer;
    analyzer.analyze(stmts);
    
    IRGenerator generator;
    auto code = generator.generate(stmts);
    
    VirtualMachine vm;
    vm.run(code);
}

void test_enum_syntax() {
    std::cout << "Testing Enum Syntax..." << std::endl;
    
    // C-style (Standard)
    std::string source1 = R"(
        enum Color {
            RED,
            GREEN,
            BLUE
        }
        print(RED);
    )";
    run_code(source1);

    // C-style
    std::string source2 = R"(
        enum Direction {
            NORTH,
            EAST,
            SOUTH,
            WEST
        }
        print(EAST);
    )";
    run_code(source2);
}

void test_enum_semantics() {
    std::cout << "Testing Enum Semantics..." << std::endl;
    
    // Comparison
    std::string source3 = R"(
        enum Status { OK, ERROR }
        let s = OK;
        if (s == OK) {
            print("Status is OK");
        }
    )";
    run_code(source3);
    
    // Strict typing (should fail if we uncomment the error case)
    /*
    std::string source4 = R"(
        enum A { X }
        enum B { Y }
        if (X == Y) { print("Fail"); }
    )";
    */
}

void test_match_enum() {
    std::cout << "Testing Match with Enums..." << std::endl;
    std::string source = R"(
        enum State { IDLE, RUNNING, STOPPED }
        let current = RUNNING;
        match (current) {
            case IDLE: print("Idle");
            case RUNNING: print("Running");
            case STOPPED: print("Stopped");
            default: print("Unknown");
        }
    )";
    run_code(source);
}

int main() {
    try {
        test_enum_syntax();
        test_enum_semantics();
        test_match_enum();
        std::cout << "All Enum tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
