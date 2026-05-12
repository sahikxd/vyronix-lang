#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Token.hpp"

namespace vyronix {

class Lexer {
public:
    explicit Lexer(std::string source, std::string filename = "unknown") noexcept;
    [[nodiscard]] std::vector<Token> tokenize();

private:
    std::string source_;
    std::string filename_;
    std::vector<Token> tokens_;
    size_t start_ = 0;
    size_t current_ = 0;
    size_t line_ = 1;
    size_t column_ = 1;

    static const std::unordered_map<std::string, TokenType> keywords_;

    [[nodiscard]] bool isAtEnd() const noexcept;
    char advance() noexcept;
    [[nodiscard]] char peek() const noexcept;
    [[nodiscard]] char peekNext() const noexcept;
    bool match(char expected) noexcept;

    void scanToken();
    void addToken(TokenType type);
    void addToken(TokenType type, TokenValue value);

    void string();
    void stringInterpolation();
    void number();
    void identifier();

    [[nodiscard]] bool isDigit(char c) const noexcept;
    [[nodiscard]] bool isAlpha(char c) const noexcept;
    [[nodiscard]] bool isAlphaNumeric(char c) const noexcept;

    void error(const std::string& message);
    
    // String interpolation state
    int interpolation_depth_ = 0;
};

} // namespace vyronix
