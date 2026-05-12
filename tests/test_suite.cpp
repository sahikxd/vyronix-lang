#include "vyronix/TestRunner.hpp"
#include "vyronix/Lexer.hpp"
#include "vyronix/Parser.hpp"
#include "vyronix/SemanticAnalyzer.hpp"
#include "vyronix/IRGenerator.hpp"
#include "vyronix/VM.hpp"
#include <sstream>

using namespace vyronix;
using namespace vyronix::test;

// --- Lexer Tests ---

VX_TEST(Lexer_Tokens) {
    Lexer lexer("let x = 10; const y = 3.14;");
    auto tokens = lexer.tokenize();
    VX_ASSERT(tokens.size() == 11); // let, x, =, 10, ;, const, y, =, 3.14, ;, EOF
    VX_ASSERT(tokens[0].type == TokenType::LET);
    VX_ASSERT(tokens[3].type == TokenType::INT_LIT);
}

VX_TEST(Lexer_Fail_UnterminatedString) {
    Lexer lexer("\"unterminated");
    VX_ASSERT_THROWS(SyntaxError, lexer.tokenize());
}

// --- Parser Tests ---

VX_TEST(Parser_Expression) {
    Lexer lexer("let x = 5 + 5;");
    Parser parser(lexer.tokenize());
    auto stmts = parser.parse();
    VX_ASSERT(stmts.size() == 1);
}

VX_TEST(Parser_Fail_MissingSemicolon) {
    Lexer lexer("let x = 5");
    Parser parser(lexer.tokenize());
    VX_ASSERT_THROWS(SyntaxError, parser.parse());
}

// --- Semantic Tests ---

VX_TEST(Semantic_TypeMismatch) {
    Lexer lexer("let x: int = \"string\";");
    Parser parser(lexer.tokenize());
    auto stmts = parser.parse();
    SemanticAnalyzer analyzer;
    VX_ASSERT_THROWS(SemanticError, analyzer.analyze(stmts));
}

VX_TEST(Semantic_ConstAssignment) {
    Lexer lexer("const x = 10; x = 20;");
    Parser parser(lexer.tokenize());
    auto stmts = parser.parse();
    SemanticAnalyzer analyzer;
    VX_ASSERT_THROWS(SemanticError, analyzer.analyze(stmts));
}

VX_TEST(Semantic_UndefinedVariable) {
    Lexer lexer("let x = y;");
    Parser parser(lexer.tokenize());
    auto stmts = parser.parse();
    SemanticAnalyzer analyzer;
    VX_ASSERT_THROWS(SemanticError, analyzer.analyze(stmts));
}

// --- VM / Execution Tests ---

std::string capture_stdout(const std::string& source) {
    std::streambuf* old = std::cout.rdbuf();
    std::stringstream ss;
    std::cout.rdbuf(ss.rdbuf());

    Lexer lexer(source);
    Parser parser(lexer.tokenize());
    auto stmts = parser.parse();
    SemanticAnalyzer analyzer;
    analyzer.analyze(stmts);
    IRGenerator generator;
    auto code = generator.generate(stmts);
    VirtualMachine vm;
    vm.run(code);

    std::cout.rdbuf(old);
    return ss.str();
}

VX_TEST(VM_Arithmetic) {
    std::string output = capture_stdout("print(10 + 20 * 2);");
    VX_ASSERT(output == "50\n");
}

VX_TEST(VM_Enums_Match) {
    std::string source = R"(
        enum Color { RED, GREEN }
        let c = GREEN;
        match (c) {
            case RED: print("red");
            case GREEN: print("green");
        }
    )";
    std::string output = capture_stdout(source);
    VX_ASSERT(output == "green\n");
}

VX_TEST(VM_Fail_Assert) {
    Lexer lexer("assert(1 == 2);");
    Parser parser(lexer.tokenize());
    auto stmts = parser.parse();
    SemanticAnalyzer analyzer;
    analyzer.analyze(stmts);
    IRGenerator generator;
    auto code = generator.generate(stmts);
    VirtualMachine vm;
    VX_ASSERT_THROWS(RuntimeError, vm.run(code));
}

int main() {
    return TestRunner::getInstance().runAll() ? 0 : 1;
}
