#include "vyronix/VM.hpp"
#include "vyronix/NativeRegistry.hpp"
#include "vyronix/Lexer.hpp"
#include "vyronix/Parser.hpp"
#include "vyronix/SemanticAnalyzer.hpp"
#include "vyronix/IRGenerator.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace vyronix;

void test_math_functions() {
    std::cout << "Testing math functions..." << std::endl;
    auto& registry = NativeRegistry::getInstance();
    registry.setupStdLib();

    // Test abs
    assert(std::get<int64_t>(registry.call("abs", {int64_t(-10)})) == 10);
    assert(std::get<double>(registry.call("abs", {double(-10.5)})) == 10.5);

    // Test sqrt
    assert(std::get<double>(registry.call("sqrt", {double(16.0)})) == 4.0);
    assert(std::get<double>(registry.call("sqrt", {int64_t(25)})) == 5.0);

    // Test pow
    assert(std::get<double>(registry.call("pow", {double(2.0), double(3.0)})) == 8.0);
    assert(std::get<double>(registry.call("pow", {int64_t(3), int64_t(2)})) == 9.0);

    std::cout << "Math functions passed!" << std::endl;
}

void test_type_checking() {
    std::cout << "Testing strict type checking..." << std::endl;
    auto& registry = NativeRegistry::getInstance();

    try {
        registry.call("sqrt", {std::string("not a number")});
        assert(false && "Should have thrown type mismatch error");
    } catch (const std::runtime_error& e) {
        std::cout << "Caught expected error: " << e.what() << std::endl;
    }

    try {
        registry.call("pow", {double(2.0)}); // Missing arg
        assert(false && "Should have thrown argument count error");
    } catch (const std::runtime_error& e) {
        std::cout << "Caught expected error: " << e.what() << std::endl;
    }

    std::cout << "Type checking passed!" << std::endl;
}

void test_file_io() {
    std::cout << "Testing file I/O..." << std::endl;
    auto& registry = NativeRegistry::getInstance();

    std::string test_file = "test_output.txt";
    std::string test_data = "Hello Vyronix!";

    // Test writeFile
    registry.call("writeFile", {test_file, test_data});

    // Test readFile
    IRValue read_back = registry.call("readFile", {test_file});
    assert(std::get<std::string>(read_back) == test_data);

    std::cout << "File I/O passed!" << std::endl;
}

void run_vyronix_assert_test(const std::string& source, bool expect_fail) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto stmts = parser.parse();
    
    SemanticAnalyzer analyzer;
    analyzer.analyze(stmts);
    
    IRGenerator generator;
    auto code = generator.generate(stmts);
    
    VirtualMachine vm;
    try {
        vm.run(code);
        if (expect_fail) throw std::runtime_error("Expected assertion failure but none occurred");
    } catch (const std::exception& e) {
        if (!expect_fail) throw;
        std::cout << "Caught expected assertion failure: " << e.what() << std::endl;
    }
}

void test_vyronix_assert() {
    std::cout << "Testing VYRONIX assert()..." << std::endl;
    
    // Pass
    run_vyronix_assert_test("assert(1 == 1);", false);
    run_vyronix_assert_test("let x = true;\nassert(x);", false);
    
    // Fail
    run_vyronix_assert_test("assert(1 == 0);", true);
    run_vyronix_assert_test("let y = false;\nassert(y);", true);
    
    std::cout << "VYRONIX assert() tests passed!" << std::endl;
}

int main() {
    try {
        test_math_functions();
        test_type_checking();
        test_file_io();
        test_vyronix_assert();
        std::cout << "All native std tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
