#include "vyronix/Lexer.hpp"
#include "vyronix/Error.hpp"
#include <iostream>

namespace vyronix {

const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"var", TokenType::VAR},
    {"let", TokenType::LET},
    {"const", TokenType::CONST},
    {"fn", TokenType::FN},
    {"proc", TokenType::PROC},
    {"if", TokenType::IF},
    {"elif", TokenType::ELIF},
    {"else", TokenType::ELSE},
    {"loop", TokenType::LOOP},
    {"for", TokenType::FOR},
    {"to", TokenType::TO},
    {"step", TokenType::STEP},
    {"match", TokenType::MATCH},
    {"case", TokenType::CASE},
    {"default", TokenType::DEFAULT},
    {"try", TokenType::TRY},
    {"catch", TokenType::CATCH},
    {"throw", TokenType::THROW},
    {"struct", TokenType::STRUCT},
    {"enum", TokenType::ENUM},
    {"import", TokenType::IMPORT},
    {"use", TokenType::USE},
    {"module", TokenType::MODULE},
    {"export", TokenType::EXPORT},
    {"return", TokenType::RETURN},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"BEGIN", TokenType::BEGIN},
    {"END", TokenType::END},
    {"call", TokenType::CALL},
    {"i8", TokenType::I8},
    {"i16", TokenType::I16},
    {"i32", TokenType::I32},
    {"i64", TokenType::I64},
    {"int", TokenType::I64},
    {"u8", TokenType::U8},
    {"u16", TokenType::U16},
    {"u32", TokenType::U32},
    {"u64", TokenType::U64},
    {"f32", TokenType::F32},
    {"f64", TokenType::F64},
    {"float", TokenType::F64},
    {"bool", TokenType::BOOL},
    {"str", TokenType::STR},
    {"string", TokenType::STR},
    {"void", TokenType::VOID},
    {"any", TokenType::ANY},
    {"array", TokenType::ARRAY},
    {"weak", TokenType::WEAK},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"null", TokenType::NULL_TOKEN},
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"not", TokenType::NOT}
};

Lexer::Lexer(std::string source, std::string filename) noexcept
    : source_(std::move(source)), filename_(std::move(filename)) {}

std::vector<Token> Lexer::tokenize() {
    while (!isAtEnd()) {
        start_ = current_;
        scanToken();
    }
    tokens_.emplace_back(TokenType::EOF_TOKEN, "", std::monostate{}, line_, column_);
    return tokens_;
}

bool Lexer::isAtEnd() const noexcept {
    return current_ >= source_.length();
}

char Lexer::advance() noexcept {
    char c = source_[current_++];
    if (c == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return c;
}

char Lexer::peek() const noexcept {
    if (isAtEnd()) return '\0';
    return source_[current_];
}

char Lexer::peekNext() const noexcept {
    if (current_ + 1 >= source_.length()) return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected) noexcept {
    if (isAtEnd()) return false;
    if (source_[current_] != expected) return false;
    current_++;
    column_++;
    return true;
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        case '(': addToken(TokenType::LPAREN); break;
        case ')': addToken(TokenType::RPAREN); break;
        case '{': 
            if (interpolation_depth_ > 0) {
                interpolation_depth_++;
            }
            addToken(TokenType::LBRACE); 
            break;
        case '}': 
            if (interpolation_depth_ > 0) {
                interpolation_depth_--;
                if (interpolation_depth_ == 0) {
                    // This is handled by stringInterpolation loop
                    return; 
                }
            }
            addToken(TokenType::RBRACE); 
            break;
        case '[': addToken(TokenType::LBRACKET); break;
        case ']': addToken(TokenType::RBRACKET); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case ':': addToken(TokenType::COLON); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '+': addToken(TokenType::PLUS); break;
        case '-': 
            addToken(match('>') ? TokenType::ARROW : TokenType::MINUS); 
            break;
        case '*': addToken(TokenType::STAR); break;
        case '%': addToken(TokenType::PERCENT); break;
        case '!':
            addToken(match('=') ? TokenType::BANG_EQ : TokenType::BANG);
            break;
        case '=':
            if (match('>')) addToken(TokenType::ARROW);
            else addToken(match('=') ? TokenType::EQ_EQ : TokenType::EQ);
            break;
        case '<':
            addToken(match('=') ? TokenType::LT_EQ : TokenType::LT);
            break;
        case '>':
            addToken(match('=') ? TokenType::GT_EQ : TokenType::GT);
            break;
        case '/':
            if (match('/')) {
                // A comment goes until the end of the line.
                while (peek() != '\n' && !isAtEnd()) advance();
            } else {
                addToken(TokenType::SLASH);
            }
            break;
        case ' ':
        case '\r':
        case '\t':
        case '\n':
            // Ignore whitespace.
            break;
        case '"': string(); break;
        default:
            if (isDigit(c)) {
                number();
            } else if (isAlpha(c)) {
                identifier();
            } else {
                error("Unexpected character.");
            }
            break;
    }
}

void Lexer::addToken(TokenType type) {
    addToken(type, std::monostate{});
}

void Lexer::addToken(TokenType type, TokenValue value) {
    std::string text = source_.substr(start_, current_ - start_);
    tokens_.emplace_back(type, text, value, line_, column_ - text.length());
}

void Lexer::string() {
    bool is_interpolated = false;
    size_t temp_current = current_;
    while (temp_current < source_.length() && source_[temp_current] != '"') {
        if (source_[temp_current] == '{') {
            is_interpolated = true;
            break;
        }
        temp_current++;
    }

    if (!is_interpolated) {
        while (peek() != '"' && !isAtEnd()) advance();
        if (isAtEnd()) {
            error("Unterminated string.");
            return;
        }
        advance(); // Closing "
        std::string value = source_.substr(start_ + 1, current_ - start_ - 2);
        addToken(TokenType::STRING_LIT, value);
    } else {
        addToken(TokenType::INTERPOLATION_START);
        stringInterpolation();
    }
}

void Lexer::stringInterpolation() {
    while (!isAtEnd()) {
        // Scan string literal part
        start_ = current_;
        while (peek() != '"' && peek() != '{' && !isAtEnd()) advance();
        
        if (isAtEnd()) {
            error("Unterminated string interpolation.");
            return;
        }

        std::string value = source_.substr(start_, current_ - start_);
        tokens_.emplace_back(TokenType::STRING_LIT, value, value, line_, column_ - value.length());

        if (peek() == '{') {
            advance(); // {
            addToken(TokenType::LBRACE);
            
            interpolation_depth_ = 1;
            while (interpolation_depth_ > 0 && !isAtEnd()) {
                start_ = current_;
                scanToken();
            }
            
            if (isAtEnd()) {
                error("Unterminated expression in string interpolation.");
                return;
            }
            
            // The '}' was consumed by scanToken and set interpolation_depth_ to 0
            // scanToken doesn't add RBRACE if it decrements depth to 0
            tokens_.emplace_back(TokenType::RBRACE, "}", std::monostate{}, line_, column_ - 1);
        } else if (peek() == '"') {
            advance(); // "
            addToken(TokenType::INTERPOLATION_END);
            return;
        }
    }
}

void Lexer::number() {
    while (isDigit(peek())) advance();

    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext())) {
        // Consume the "."
        advance();

        while (isDigit(peek())) advance();
        
        double value = std::stod(source_.substr(start_, current_ - start_));
        addToken(TokenType::FLOAT_LIT, value);
    } else {
        int64_t value = std::stoll(source_.substr(start_, current_ - start_));
        addToken(TokenType::INT_LIT, value);
    }
}

void Lexer::identifier() {
    while (isAlphaNumeric(peek())) advance();

    std::string text = source_.substr(start_, current_ - start_);
    auto it = keywords_.find(text);
    TokenType type = (it != keywords_.end()) ? it->second : TokenType::IDENTIFIER;
    
    if (type == TokenType::TRUE) addToken(type, true);
    else if (type == TokenType::FALSE) addToken(type, false);
    else if (type == TokenType::NULL_TOKEN) addToken(type, std::monostate{});
    else addToken(type);
}

bool Lexer::isDigit(char c) const noexcept {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) const noexcept {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isAlphaNumeric(char c) const noexcept {
    return isAlpha(c) || isDigit(c);
}

void Lexer::error(const std::string& message) {
    throw SyntaxError(message, filename_, line_, column_);
}

} // namespace vyronix