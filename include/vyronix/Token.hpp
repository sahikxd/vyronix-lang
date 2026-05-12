#pragma once

#include <string>
#include <variant>
#include <cstdint>
#include "TokenType.hpp"

namespace vyronix {

using TokenValue = std::variant<std::monostate, int64_t, double, std::string, bool>;

struct Token {
    TokenType type;
    std::string lexeme;
    TokenValue value;
    size_t line;
    size_t column;

    Token(TokenType type, std::string lexeme, TokenValue value, size_t line, size_t column)
        : type(type), lexeme(std::move(lexeme)), value(std::move(value)), line(line), column(column) {}
};

} // namespace vyronix
