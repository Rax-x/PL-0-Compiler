#include "tokenizer.hpp"

#include <cctype>

namespace pl0::tokenizer {

auto Tokenizer::tokenize() -> std::vector<Token> {

    while(!isAtEnd()){
        m_start = m_curr;
        scanToken();
    }

    makeToken(TokenType::Eof);

    return m_tokens;
}

auto Tokenizer::scanToken() -> void {

    const char c = advance();

    switch(c) {
        case '\n':
            m_line++;
            [[fallthrough]];
        case ' ':
            [[fallthrough]];
        case '\r':
            [[fallthrough]];
        case '\t':
            break;
        case '.':
            makeToken(TokenType::Dot);
            break;
        case '=':
            makeToken(TokenType::Equal);
            break;
        case ',':
            makeToken(TokenType::Comma);
            break;
        case ';':
            makeToken(TokenType::Semicolon);
            break;
        case ':':
            makeToken(match('=') 
                        ? TokenType::Assign
                        : TokenType::UnexpectedCharacter);
            break;
        case '?':
            makeToken(TokenType::QuestionMark);
            break;
        case '!':
            makeToken(TokenType::ExclamationMark);
            break;
        case '#':
            makeToken(TokenType::NotEqual);
            break;
        case '<':
            makeToken(match('=') 
                        ? TokenType::LessEqual
                        : TokenType::Less);
            break;
        case '>':
            makeToken(match('=') 
                        ? TokenType::GreaterEqual
                        : TokenType::Greater);
            break;
        case '+':
            makeToken(TokenType::Plus);
            break;
        case '-':
            makeToken(TokenType::Minus);
            break;
        case '*':
            makeToken(TokenType::Star);
            break;
        case '/':
            makeToken(TokenType::Slash);
            break;
        case '(':
            makeToken(TokenType::LeftParen);
            break;
        case ')':
            makeToken(TokenType::RightParen);
            break;
        default: {

            if(std::isdigit(c)){
                while(std::isdigit(peek())) advance();
                makeToken(TokenType::Number);

                break;
            }

            if(std::isalpha(c)) {
                while(std::isalnum(peek())) advance();

                // Check if the current identifer is a keyword
                const auto lexeme = m_source.substr(m_start, m_curr - m_start);
                const auto entry = m_keywords.find(lexeme);

                makeToken(entry != m_keywords.end()
                            ? entry->second
                            : TokenType::Identifier);

                break;
            }

            makeToken(TokenType::UnexpectedCharacter);
            break;
        }
    }

}

}
