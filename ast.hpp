#ifndef _AST_HPP_
#define _AST_HPP_

#include <memory>
#include <type_traits>
#include <vector>

#include "token.hpp"

namespace pl0::ast {

using token::Token;

// Statement base class

struct Statement {
    Statement() = default;
    virtual ~Statement() = default;
};

using StatementPtr = std::unique_ptr<Statement>;

template<typename T>
concept StatementType = requires { 
    std::is_base_of_v<Statement, T>(); 
};

template <StatementType Stmt, typename... Args>
inline auto buildStatement(Args&&... args) -> StatementPtr {
    return std::unique_ptr<Statement>(new Stmt(std::forward<Args>(args)...));
}

// Expression base class

struct Expression {
    Expression() = default;
    virtual ~Expression() = default;
};

using ExpressionPtr = std::unique_ptr<Expression>;

template<typename T>
concept ExpressionType = requires { 
    std::is_base_of_v<Expression, T>(); 
};

template <ExpressionType Expr, typename... Args>
inline auto buildExpression(Args&&... args) -> ExpressionPtr {
    return std::unique_ptr<Expression>(new Expr(std::forward<Args>(args)...));
}

// Statements

struct Block final : public Statement {

    Block(StatementPtr& constantsDeclaration,
          StatementPtr& variablesDeclaration,
          std::vector<StatementPtr>& procedureDeclarations,
          StatementPtr statement) 
        : constantsDeclaration(std::move(constantsDeclaration)),
          variablesDeclaration(std::move(variablesDeclaration)),
          procedureDeclarations(std::move(procedureDeclarations)),
          statement(std::move(statement)) {}
          
    StatementPtr constantsDeclaration;
    StatementPtr variablesDeclaration;
    std::vector<StatementPtr> procedureDeclarations;

    StatementPtr statement;
};

struct ConstDeclaration {
    Token identifier;
    int initializer;
};

struct ConstDeclarations final : public Statement {

    ConstDeclarations(std::vector<ConstDeclaration>& declarations)
        : declarations(std::move(declarations)) {}

    std::vector<ConstDeclaration> declarations;
};

struct VariableDeclarations final : public Statement {
    VariableDeclarations(std::vector<Token>& identifiers)
        : identifiers(std::move(identifiers)) {}

    std::vector<Token> identifiers;
};

struct ProcedureDeclaration final : public Statement {
    
    ProcedureDeclaration(Token name, StatementPtr& block)
        : name(name), block(std::move(block)) {}

    Token name;
    StatementPtr block;
};

struct AssignStatement final : public Statement {

    AssignStatement(Token lvalue, ExpressionPtr& rvalue)
        : lvalue(lvalue), rvalue(std::move(rvalue)) {}

    Token lvalue;
    ExpressionPtr rvalue;
};

struct CallStatement final : public Statement {
    CallStatement(Token callee)
        : callee(callee) {}

    Token callee;
};

struct InputStatement final : public Statement {
    InputStatement(Token destination)
        : destination(destination) {}

    Token destination;
};

struct PrintStatement final : public Statement {
    PrintStatement(Token argument)
        : argument(argument) {}

    Token argument;
};

struct BeginStatement final : public Statement {
    BeginStatement(std::vector<StatementPtr>& statements)
        : statements(std::move(statements)) {}

    std::vector<StatementPtr> statements;
};

struct IfStatement final : public Statement {
    
    IfStatement(ExpressionPtr& condition, StatementPtr& body)
        : condition(std::move(condition)),
          body(std::move(body)) {}
          

    ExpressionPtr condition;
    StatementPtr body;
};

struct WhileStatement final : public Statement {
    
    WhileStatement(ExpressionPtr& condition, StatementPtr& body)
        : condition(std::move(condition)),
          body(std::move(body)) {}

    ExpressionPtr condition;
    StatementPtr body;
};

// Expressions

struct OddExpression final : public Expression {
    OddExpression(ExpressionPtr& expr)
        : expr(std::move(expr)) {}

    ExpressionPtr expr;
};

struct BinaryExpression final : public Expression {
    BinaryExpression(ExpressionPtr& left, Token op, ExpressionPtr& right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    ExpressionPtr left;
    Token op;
    ExpressionPtr right;
};

struct UnaryExpression final : public Expression {
    UnaryExpression(Token op, ExpressionPtr& right)
        : op(op), right(std::move(right)) {}

    Token op;
    ExpressionPtr right;
};

struct VariableExpression final : public Expression {
    VariableExpression(Token name)
        : name(name) {}

    Token name;
};

struct LiteralExpression final : public Expression {
    constexpr LiteralExpression(int value)
        : value(value) {}

    int value;
};

}

#endif
