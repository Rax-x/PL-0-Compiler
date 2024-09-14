#ifndef _TOKEN_HPP_
#define _TOKEN_HPP_

#include <cstdint>
#include <string_view>

namespace pl0::token {

enum class TokenType : std::uint8_t {

    // Literals
    Identifier,
    Number,

    // Symbols
    Dot,
    Equal,
    Comma,
    Semicolon,
    Assign,
    QuestionMark,
    ExclamationMark,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Plus,
    Minus,
    Star,
    Slash,
    LeftParen,
    RightParen,
    
    // Keywords
    ConstKeyword,
    VarKeyword,
    ProcedureKeyword,
    CallKeyword,
    BeginKeyword,
    EndKeyword,
    IfKeyword,
    ThenKeyword,
    WhileKeyword,
    DoKeyword,
    OddKeyword,

    // Errors
    UnexpectedCharacter,
    
    Eof
};

struct Token final {

    Token() = default;
    Token(TokenType type, std::string_view lexeme, std::uint32_t line)
        : type(type), lexeme(lexeme), line(line) {}

    TokenType type;
    std::string_view lexeme;
    std::uint32_t line;
};

}

#endif
