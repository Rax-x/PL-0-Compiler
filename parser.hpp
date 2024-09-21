#ifndef _PARSER_HPP_
#define _PARSER_HPP_

#include <initializer_list>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>
#include <format>

#include "errors_holder_trait.hpp"
#include "token.hpp"
#include "ast.hpp"

namespace pl0::parser {

using namespace error;
using namespace token;
using namespace ast;

class Parser final : public ErrorsHolderTrait {
public:
    explicit Parser(std::vector<Token>& tokens)
        : m_tokens(std::move(tokens)),
          m_curr(0),
          m_panicMode(false) {}
          
    auto parseProgram() -> StatementPtr;

private:

    auto block() -> StatementPtr;
    auto constDeclarations() -> StatementPtr;
    auto variableDeclarations() -> StatementPtr;
    auto procedureDeclaration() -> StatementPtr;

    auto statement() -> StatementPtr;
    auto assignStatement() -> StatementPtr;
    auto callStatement() -> StatementPtr;
    auto inputStatement() -> StatementPtr;
    auto printStatement() -> StatementPtr;
    auto beginStatement() -> StatementPtr;
    auto ifStatement() -> StatementPtr;
    auto whileStatement() -> StatementPtr;

    auto condition() -> ExpressionPtr;

    auto expression() -> ExpressionPtr;
    auto termExpression() -> ExpressionPtr;
    auto factorExpression() -> ExpressionPtr;
    
    auto convertToInteger(Token name) -> std::optional<int>;

    [[nodiscard]] 
    inline auto previous() const -> const Token& {
        return m_tokens[m_curr - 1];
    }

    [[nodiscard]] 
    inline auto current() const -> const Token& {
        return m_tokens[m_curr];
    }

    constexpr auto isAtEnd() const -> bool {
        return m_curr >= m_tokens.size();
    }

    inline auto advance() -> void {
        if(isAtEnd()) return;
        m_curr++;
    }
    
    auto match(const std::initializer_list<TokenType>& types) -> bool;
    auto consume(TokenType type, const char* message) -> std::optional<Token>;
    auto synchronize() -> void;

    template<typename... Args>
    inline auto error(std::string_view fmt, Args&&... args) -> void {
        pushError(std::vformat(fmt, std::make_format_args(args...)));
        m_panicMode = true;
    }

private:
    std::vector<Token> m_tokens;
    std::uint32_t m_curr;

    bool m_panicMode;
};


}

#endif
