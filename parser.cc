#include "parser.hpp"
#include "ast.hpp"
#include "token.hpp"

#include <charconv>
#include <system_error>

namespace pl0::parser {

auto Parser::parseProgram() -> StatementPtr {

    StatementPtr program = block();
    
    if(!consume(TokenType::Dot, "Expect '.' at end of the program.").has_value()) {
        return nullptr;
    }

    if(!consume(TokenType::Eof, "Unterminated file.").has_value()) {
        return nullptr;
    }

    return program;
}

auto Parser::block() -> StatementPtr {

    StatementPtr constants = match({TokenType::ConstKeyword})
        ? constDeclarations()
        : nullptr;

    StatementPtr variables = match({TokenType::VarKeyword})
        ? variableDeclarations()
        : nullptr;
    
    std::vector<StatementPtr> procedures;
    while(match({TokenType::ProcedureKeyword})){
        procedures.push_back(procedureDeclaration());
    }

    StatementPtr stmt = statement();

    return buildStatement<Block>(constants, variables, procedures, stmt);
}

auto Parser::constDeclarations() -> StatementPtr {

    std::vector<ConstDeclaration> declarations;

    do {
        auto ident = consume(TokenType::Identifier, "Expect constant name.");
        if(!ident.has_value()) return nullptr;
        
        if(!consume(TokenType::Equal, "Expect '=' after constant name.").has_value()) {
            return nullptr;
        }

        if(!consume(TokenType::Number, "Expect '=' after constant value.").has_value()) {
            return nullptr;
        }

        auto result = convertToInteger(previous());
        if(!result.has_value()) return nullptr;

        declarations.emplace_back(ident.value(), result.value());
    } while(match({TokenType::Comma}));

    if(!consume(TokenType::Semicolon, "Expect ';' after constant declarations.").has_value()) {
        return nullptr;
    }

    return buildStatement<ConstDeclarations>(declarations);
}

auto Parser::variableDeclarations() -> StatementPtr { 

    std::vector<Token> identifiers;

    do {
        auto ident = consume(TokenType::Identifier, "Expect constant name.");
        if(!ident.has_value()) return nullptr;
        
        identifiers.push_back(ident.value());
    } while(match({TokenType::Comma}));

    if(!consume(TokenType::Semicolon, "Expect ';' after variable declarations.").has_value()) {
        return nullptr;
    }

    return buildStatement<VariableDeclarations>(identifiers);
}

auto Parser::procedureDeclaration() -> StatementPtr { 
    
    auto name = consume(TokenType::Identifier, "Expect procedure name.");
    if(!name.has_value()) return nullptr;

    if(!consume(TokenType::Semicolon, "Expect ';' before procedure body.").has_value()) {
        return nullptr;
    }

    StatementPtr body = block();

    if(!consume(TokenType::Semicolon, "Expect ';' at end of procedure body.").has_value()) {
        return nullptr;
    }

    return buildStatement<ProcedureDeclaration>(name.value(), body); 
}

auto Parser::statement() -> StatementPtr {

    if(m_panicMode) synchronize();

    if(match({TokenType::Identifier})) {
        return assignStatement();
    } else if(match({TokenType::CallKeyword})) {
        return callStatement();
    } else if(match({TokenType::QuestionMark})){
        return inputStatement();
    } else if(match({TokenType::ExclamationMark})) {
        return printStatement();
    } else if(match({TokenType::BeginKeyword})) {
        return beginStatement();
    } else if(match({TokenType::IfKeyword})) {
        return ifStatement();
    } else if(match({TokenType::WhileKeyword})) {
        return whileStatement();
    }

    error("[Ln: {}] Error: Invalid statement.", current().line);
    return nullptr;
}

auto Parser::assignStatement() -> StatementPtr{
    auto ident = previous();

    if(!consume(TokenType::Assign, "Expect ':=' after lvalue.").has_value()) {
        return nullptr;
    }

    ExpressionPtr rvalue = expression();
    return buildStatement<AssignStatement>(ident, rvalue);
}

auto Parser::callStatement() -> StatementPtr {

    auto ident = consume(TokenType::Identifier, "Expect the procedure name after 'call'.");

    return ident.has_value()
        ? buildStatement<CallStatement>(ident.value())
        : nullptr;
}

auto Parser::inputStatement() -> StatementPtr{

    auto ident = consume(TokenType::Identifier, "Expect an identifier.");

    return ident.has_value()
        ? buildStatement<InputStatement>(ident.value())
        : nullptr;
}

auto Parser::printStatement() -> StatementPtr {
    auto expr = expression();
    return buildStatement<PrintStatement>(expr);
}

auto Parser::beginStatement() -> StatementPtr{

    std::vector<StatementPtr> statements;

    do {
        statements.push_back(statement());
    } while(match({TokenType::Semicolon}));
    
    if(!consume(TokenType::EndKeyword, "Expect 'end' after statements.").has_value()) {
        return nullptr;
    }
    
    return buildStatement<BeginStatement>(statements);
}
auto Parser::ifStatement() -> StatementPtr{

    ExpressionPtr cond = condition();

    if(!consume(TokenType::ThenKeyword, "Expect 'then' after condition.").has_value()) {
        return nullptr;
    }

    StatementPtr body = statement();

    return buildStatement<IfStatement>(cond, body);
}

auto Parser::whileStatement() -> StatementPtr{

    ExpressionPtr cond = condition();

    if(!consume(TokenType::DoKeyword, "Expect 'do' after condition.").has_value()) {
        return nullptr;
    }

    StatementPtr body = statement();

    return buildStatement<WhileStatement>(cond, body);
}

auto Parser::condition() -> ExpressionPtr {

    if(match({TokenType::OddKeyword})){
        ExpressionPtr expr = expression();
        return buildExpression<OddExpression>(expr);
    }

    ExpressionPtr left = expression();

    if (!match({TokenType::Equal, TokenType::NotEqual, TokenType::Less,
                TokenType::LessEqual, TokenType::Greater,
                TokenType::GreaterEqual})) {

      error("[Ln {}] Error: Expect one of these operators: '=', '#', '<', '<=', '>', '>='.", current().line);
      return nullptr;
    }

    Token op = previous();
    ExpressionPtr right = expression();

    return  buildExpression<BinaryExpression>(left, op, right);
}

auto Parser::expression() -> ExpressionPtr { 

    std::optional<Token> unaryOperator = {};

    if(match({TokenType::Plus, TokenType::Minus})){
        unaryOperator = previous();
    }

    ExpressionPtr left = termExpression();
    while(match({TokenType::Plus, TokenType::Minus})){
        Token op = previous();
        ExpressionPtr right = termExpression();

        left = buildExpression<BinaryExpression>(left, op, right);
    }

    if(unaryOperator.has_value()) {
        left = buildExpression<UnaryExpression>(unaryOperator.value(), left);
    }
    
    return left;
}

auto Parser::termExpression() -> ExpressionPtr {
    ExpressionPtr left = factorExpression();

    while(match({TokenType::Star, TokenType::Slash})){
        Token op = previous();
        ExpressionPtr right = factorExpression();

        left = buildExpression<BinaryExpression>(left, op, right);
    }

    return left;
}

auto Parser::factorExpression() -> ExpressionPtr { 

    if(match({TokenType::Identifier})) {
        return buildExpression<VariableExpression>(previous());
    }

    if(match({TokenType::Number})){
        auto result = convertToInteger(previous());

        return result.has_value() 
            ? buildExpression<LiteralExpression>(result.value())
            : nullptr;
    }
    
    if(match({TokenType::LeftParen})){
        ExpressionPtr expr = expression();
        if(consume(TokenType::RightParen, "Expect a ')' after the expression.").has_value()){
            return expr;
        }

        return nullptr;
    }
    
    error("[Ln: {}] Error: Invalid expression '{}'.", current().line, current().lexeme);
    return nullptr; 
}

auto Parser::convertToInteger(Token token) -> std::optional<int> {

    const auto& lexeme = token.lexeme;

    int result;
    const auto& [_, err] = std::from_chars(lexeme.data(), 
                                           lexeme.data() + lexeme.size(), 
                                           result);
    switch(err) {
        case std::errc::invalid_argument:
            error("[Ln: {}] Error: This isn't a valid integer value '{}'.", token.line, lexeme);
            return {};
        case std::errc::result_out_of_range:
            error("[Ln: {}] Error: This literal is larger than an interger '{}'.", token.line, lexeme);
            return {};
        default:
            break;
    }

    return result;
}

auto Parser::match(const std::initializer_list<TokenType>& types) -> bool {
    for(TokenType type : types){
        if(current().type == type){
            return (advance(), true);
        }
    }

    return false;
}

auto Parser::consume(TokenType type, const char* message) -> std::optional<Token> {
    
    if(match({type})){
        return previous();
    }

    error("[Ln: {}] Error: {}", current().line, message);
    return {};
}


auto Parser::synchronize() -> void {

    while(!isAtEnd()){

        switch(current().type){
            case TokenType::ConstKeyword:
                [[fallthrough]];
            case TokenType::VarKeyword:
                [[fallthrough]];
            case TokenType::ProcedureKeyword:
                [[fallthrough]];
            case TokenType::CallKeyword:
                [[fallthrough]];
            case TokenType::BeginKeyword:
                [[fallthrough]];
            case TokenType::EndKeyword:
                [[fallthrough]];
            case TokenType::IfKeyword:
                [[fallthrough]];
            case TokenType::ThenKeyword:
                [[fallthrough]];
            case TokenType::WhileKeyword:
                [[fallthrough]];
            case TokenType::DoKeyword:
                [[fallthrough]];
            case TokenType::QuestionMark:
                [[fallthrough]];
            case TokenType::ExclamationMark:
                [[fallthrough]];
            case TokenType::Identifier:
                return;
            default:
                advance();
                break;
        }
    }
    
}

}
