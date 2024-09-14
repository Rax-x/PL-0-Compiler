#ifndef _TOKENIZER_HPP_
#define _TOKENIZER_HPP_

#include <cstdint>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "token.hpp"

namespace pl0::tokenizer {

using namespace token;

class Tokenizer final {
public:
    explicit Tokenizer(std::string_view source) 
        : m_source(source) {}

    auto tokenize() -> std::vector<Token>;

private:

    auto scanToken() -> void;

    inline auto advance() -> char {
        return !isAtEnd()
            ? m_source[m_curr++]
            : '\0';
    }

    inline auto match(char c) -> bool {
        if(peek() == c) {
            return advance(), true;
        }

        return false;
    }

    constexpr auto isAtEnd() const -> bool {
        return m_curr >= m_source.length();
    }

    constexpr auto peek() const -> char {
        return m_source[m_curr];
    }
    
    inline auto makeToken(TokenType type) -> void {
        const auto lexemeLength = m_curr - m_start;
        m_tokens.emplace_back(type, m_source.substr(m_start, lexemeLength), m_line);
    }

private:

    std::string_view m_source;
    std::vector<Token> m_tokens;

    const std::unordered_map<std::string_view, TokenType> m_keywords = {
        {"const", TokenType::ConstKeyword},
        {"var", TokenType::VarKeyword},
        {"procedure", TokenType::ProcedureKeyword},
        {"call", TokenType::CallKeyword},
        {"begin", TokenType::BeginKeyword},
        {"end", TokenType::EndKeyword},
        {"if", TokenType::IfKeyword},
        {"then", TokenType::ThenKeyword},
        {"while", TokenType::WhileKeyword},
        {"do", TokenType::DoKeyword},
        {"odd", TokenType::OddKeyword}
    };

    std::uint32_t m_curr = 0;
    std::uint32_t m_start = 0;

    std::uint32_t m_line = 1;
};

}

#endif
