#pragma once

#include <vector>
#include <string>
#include <memory>
#include <set>
#include <map>
#include "Token.hpp"
#include "AST.hpp"
#include "Error.hpp"

namespace vyronix {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens, std::string filename = "unknown") noexcept;
    [[nodiscard]] std::vector<std::unique_ptr<Stmt>> parse();

    static std::set<std::string> imported_files;
    static std::map<std::string, std::vector<std::unique_ptr<Stmt>>> module_cache;

private:
    std::vector<Token> tokens_;
    std::string filename_;
    size_t current_ = 0;
    bool panicMode_ = false;
    int errorCount_ = 0;
    const int maxErrors_ = 50;

    // Statements
    [[nodiscard]] std::unique_ptr<Stmt> declaration();
    [[nodiscard]] std::unique_ptr<Stmt> variableDeclaration();
    [[nodiscard]] std::unique_ptr<Stmt> constDeclaration();
    [[nodiscard]] std::unique_ptr<Stmt> functionDeclaration();
    [[nodiscard]] std::unique_ptr<Stmt> procDeclaration();
    [[nodiscard]] std::unique_ptr<Stmt> structDeclaration();
    [[nodiscard]] std::unique_ptr<Stmt> classDeclaration();
    [[nodiscard]] std::unique_ptr<Stmt> enumDeclaration();
    [[nodiscard]] std::unique_ptr<Stmt> importDeclaration();
    
    [[nodiscard]] std::unique_ptr<Stmt> statement();
    [[nodiscard]] std::unique_ptr<Stmt> ifStatement();
    [[nodiscard]] std::unique_ptr<Stmt> loopStatement();
    [[nodiscard]] std::unique_ptr<Stmt> forStatement();
    [[nodiscard]] std::unique_ptr<Stmt> matchStatement();
    [[nodiscard]] std::unique_ptr<Stmt> tryStatement();
    [[nodiscard]] std::unique_ptr<Stmt> throwStatement();
    [[nodiscard]] std::unique_ptr<Stmt> returnStatement();
    [[nodiscard]] std::unique_ptr<Stmt> breakStatement();
    [[nodiscard]] std::unique_ptr<Stmt> continueStatement();
    [[nodiscard]] std::unique_ptr<Stmt> expressionStatement();
    [[nodiscard]] std::unique_ptr<BlockStmt> block();

    // Expressions
    [[nodiscard]] std::unique_ptr<Expr> expression();
    [[nodiscard]] std::unique_ptr<Expr> assignment();
    [[nodiscard]] std::unique_ptr<Expr> logical_or();
    [[nodiscard]] std::unique_ptr<Expr> logical_and();
    [[nodiscard]] std::unique_ptr<Expr> equality();
    [[nodiscard]] std::unique_ptr<Expr> comparison();
    [[nodiscard]] std::unique_ptr<Expr> term();
    [[nodiscard]] std::unique_ptr<Expr> factor();
    [[nodiscard]] std::unique_ptr<Expr> unary();
    [[nodiscard]] std::unique_ptr<Expr> primary();
    
    [[nodiscard]] std::unique_ptr<Expr> finishCall(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Type>> type_args);
    [[nodiscard]] std::unique_ptr<Expr> arrayAccess(std::unique_ptr<Expr> object);
    [[nodiscard]] std::unique_ptr<Expr> interpolatedString();

    // Types
    [[nodiscard]] std::unique_ptr<Type> parseType();

    // Helpers
    bool match(std::initializer_list<TokenType> types) noexcept;
    [[nodiscard]] bool check(TokenType type) const noexcept;
    Token advance() noexcept;
    [[nodiscard]] bool isAtEnd() const noexcept;
    [[nodiscard]] Token peek() const noexcept;
    [[nodiscard]] Token previous() const noexcept;
    Token consume(TokenType type, const std::string& message);
    void synchronize() noexcept;
    SyntaxError error(const Token& token, const std::string& message) noexcept;
};

} // namespace vyronix
