#pragma once

namespace vyronix {

enum class TokenType {
    // Keywords
    VAR, LET, CONST, FN, PROC, IF, ELIF, ELSE, LOOP, FOR, TO, STEP, 
    MATCH, CASE, DEFAULT, TRY, CATCH, THROW, STRUCT, ENUM, 
    IMPORT, USE, MODULE, EXPORT, RETURN, BREAK, CONTINUE,
    BEGIN, END, CALL,

    // Types
    I8, I16, I32, I64,
    U8, U16, U32, U64,
    F32, F64,
    BOOL, STR, VOID, ANY,
    ARRAY, WEAK,

    // Literals
    IDENTIFIER, STRING_LIT, INT_LIT, FLOAT_LIT, TRUE, FALSE, NULL_TOKEN,

    // Operators
    PLUS, MINUS, STAR, SLASH, PERCENT,
    EQ, EQ_EQ, BANG, BANG_EQ,
    LT, LT_EQ, GT, GT_EQ,
    AND, OR, NOT,

    // Delimiters
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    COLON, COMMA, SEMICOLON, DOT, ARROW,

    // Special
    INTERPOLATION_START, INTERPOLATION_END,
    EOF_TOKEN, ERROR
};

} // namespace vyronix
