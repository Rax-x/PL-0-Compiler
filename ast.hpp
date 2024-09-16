#ifndef _AST_HPP_
#define _AST_HPP_

#include <memory>
#include <type_traits>
#include <vector>

#include "token.hpp"

namespace pl0::ast {

using token::Token;

// Forward

struct Block;
struct ConstDeclarations;
struct VariableDeclarations;
struct ProcedureDeclaration;

struct AssignStatement;
struct CallStatement;
struct InputStatement;
struct PrintStatement;
struct BeginStatement;
struct IfStatement;
struct WhileStatement;

struct OddExpression;
struct BinaryExpression;
struct UnaryExpression;
struct VariableExpression;
struct LiteralExpression;

struct AstVisitor {
    AstVisitor() = default;
    virtual ~AstVisitor() = default;

    virtual auto visit(Block* block) -> void = 0;
    virtual auto visit(ConstDeclarations* decl) -> void = 0;
    virtual auto visit(VariableDeclarations* decl) -> void = 0;
    virtual auto visit(ProcedureDeclaration* decl) -> void = 0;

    virtual auto visit(AssignStatement* stmt) -> void = 0;
    virtual auto visit(CallStatement* stmt) -> void = 0;
    virtual auto visit(InputStatement* stmt) -> void = 0;
    virtual auto visit(PrintStatement* stmt) -> void = 0;
    virtual auto visit(BeginStatement* stmt) -> void = 0;
    virtual auto visit(IfStatement* stmt) -> void = 0;
    virtual auto visit(WhileStatement* stmt) -> void = 0;
    
    virtual auto visit(OddExpression* expr) -> void = 0;
    virtual auto visit(BinaryExpression* expr) -> void = 0;
    virtual auto visit(UnaryExpression* expr) -> void = 0;
    virtual auto visit(VariableExpression* expr) -> void = 0;
    virtual auto visit(LiteralExpression* expr) -> void = 0;
};

// Statement base class

struct Statement {
    Statement() = default;
    virtual ~Statement() = default;

    virtual auto accept(AstVisitor* visitor) -> void = 0;
};

using StatementPtr = std::unique_ptr<Statement>;

template<typename T>
concept StatementType = requires { 
    std::is_base_of_v<Statement, T>; 
};

template <StatementType Stmt, typename... Args>
inline auto buildStatement(Args&&... args) -> StatementPtr {
    return std::unique_ptr<Statement>(new Stmt(std::forward<Args>(args)...));
}

// Expression base class

struct Expression {
    Expression() = default;
    virtual ~Expression() = default;

    virtual auto accept(AstVisitor* visitor) -> void = 0;
};

using ExpressionPtr = std::unique_ptr<Expression>;

template<typename T>
concept ExpressionType = requires { 
    std::is_base_of_v<Expression, T>; 
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
          StatementPtr& statement) 
        : constantsDeclaration(std::move(constantsDeclaration)),
          variablesDeclaration(std::move(variablesDeclaration)),
          procedureDeclarations(std::move(procedureDeclarations)),
          statement(std::move(statement)) {}
          
    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }
    

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

    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    std::vector<ConstDeclaration> declarations;
};

struct VariableDeclarations final : public Statement {
    VariableDeclarations(std::vector<Token>& identifiers)
        : identifiers(std::move(identifiers)) {}

    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    std::vector<Token> identifiers;
};

struct ProcedureDeclaration final : public Statement {
    
    ProcedureDeclaration(Token name, StatementPtr& block)
        : name(name), block(std::move(block)) {}

    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    Token name;
    StatementPtr block;
};

struct AssignStatement final : public Statement {

    AssignStatement(Token lvalue, ExpressionPtr& rvalue)
        : lvalue(lvalue), rvalue(std::move(rvalue)) {}

    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    Token lvalue;
    ExpressionPtr rvalue;
};

struct CallStatement final : public Statement {
    CallStatement(Token callee)
        : callee(callee) {}

    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    Token callee;
};

struct InputStatement final : public Statement {
    InputStatement(Token destination)
        : destination(destination) {}

    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    Token destination;
};

struct PrintStatement final : public Statement {
    PrintStatement(Token argument)
        : argument(argument) {}

    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    Token argument;
};

struct BeginStatement final : public Statement {
    BeginStatement(std::vector<StatementPtr>& statements)
        : statements(std::move(statements)) {}

    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    std::vector<StatementPtr> statements;
};

struct IfStatement final : public Statement {
    
    IfStatement(ExpressionPtr& condition, StatementPtr& body)
        : condition(std::move(condition)),
          body(std::move(body)) {}
          
    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    ExpressionPtr condition;
    StatementPtr body;
};

struct WhileStatement final : public Statement {
    
    WhileStatement(ExpressionPtr& condition, StatementPtr& body)
        : condition(std::move(condition)),
          body(std::move(body)) {}

    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    ExpressionPtr condition;
    StatementPtr body;
};

// Expressions

struct OddExpression final : public Expression {
    OddExpression(ExpressionPtr& expr)
        : expr(std::move(expr)) {}

    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    ExpressionPtr expr;
};

struct BinaryExpression final : public Expression {
    BinaryExpression(ExpressionPtr& left, Token op, ExpressionPtr& right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    ExpressionPtr left;
    Token op;
    ExpressionPtr right;
};

struct UnaryExpression final : public Expression {
    UnaryExpression(Token op, ExpressionPtr& right)
        : op(op), right(std::move(right)) {}


    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    Token op;
    ExpressionPtr right;
};

struct VariableExpression final : public Expression {
    VariableExpression(Token name)
        : name(name) {}

    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    Token name;
};

struct LiteralExpression final : public Expression {
    constexpr LiteralExpression(int value)
        : value(value) {}

    auto accept(AstVisitor* visitor) -> void {
        visitor->visit(this);
    }

    int value;
};

// AST Printer

class AstPrinter : public AstVisitor {
public:
    AstPrinter() = default;

    auto print(StatementPtr& ast) -> void;

private:
    auto visit(Block* block) -> void;
    auto visit(ConstDeclarations* decl) -> void;
    auto visit(VariableDeclarations* decl) -> void;
    auto visit(ProcedureDeclaration* decl) -> void;

    auto visit(AssignStatement* stmt) -> void;
    auto visit(CallStatement* stmt) -> void;
    auto visit(InputStatement* stmt) -> void;
    auto visit(PrintStatement* stmt) -> void;
    auto visit(BeginStatement* stmt) -> void;
    auto visit(IfStatement* stmt) -> void;
    auto visit(WhileStatement* stmt) -> void;
    
    auto visit(OddExpression* expr) -> void;
    auto visit(BinaryExpression* expr) -> void;
    auto visit(UnaryExpression* expr) -> void;
    auto visit(VariableExpression* expr) -> void;
    auto visit(LiteralExpression* expr) -> void;

    inline auto indent() -> void {
        m_level++;
    }

    inline auto dedent() -> void {
        m_level--;
    }

    auto newline() const -> void;
private:
    static constexpr int TAB_SIZE = 2;
    int m_level = 0;
};

}

#endif
