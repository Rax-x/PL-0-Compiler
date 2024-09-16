#include "ast.hpp"

#include <iostream>

namespace pl0::ast {

auto AstPrinter::print(StatementPtr& ast) -> void {
    ast->accept(this);
    std::cout << "\n\n";
}

auto AstPrinter::newline() const -> void {

    std::cout << '\n';
    for(int i = 0; i < m_level * TAB_SIZE; i++){
        std::cout << ' ';
    }
}

auto AstPrinter::visit(Block* block) -> void {

    std::cout << "Block:";
    
    indent();
    
    if(block->constantsDeclaration != nullptr) {
        newline();
        std::cout << "Constants:";
        indent();
        newline();

        block->constantsDeclaration->accept(this);
        dedent();

    }

    if(block->variablesDeclaration != nullptr) {
        newline();
        std::cout << "Variables:";
        indent();
        newline();

        block->variablesDeclaration->accept(this);
        dedent();

    }

    if(!block->procedureDeclarations.empty()) {
        newline();
        std::cout << "Procedures:";
        indent();

        for(const auto& proc : block->procedureDeclarations) {
            newline();
            proc->accept(this);
        }

        dedent();
    }

    newline();
    std::cout << "Statement:";
    
    indent();
    newline();
    block->statement->accept(this);
    dedent();

    dedent();
}

auto AstPrinter::visit(ConstDeclarations* decl) -> void {

    std::cout << "BeginStatement:";
    
    indent();

    for(const auto& [name, value] : decl->declarations) {
        newline();
        std::cout << name.lexeme << " = " << value;
    }

    dedent();
}

auto AstPrinter::visit(VariableDeclarations* decl) -> void {

    std::cout << "VariableDeclarations: ";
    
    for(const auto& ident : decl->identifiers){
        std::cout << ident.lexeme << ' ';
    }
}

auto AstPrinter::visit(ProcedureDeclaration* decl) -> void {
    std::cout << "ProcedureDeclaration:";
    indent();
    newline();

    std::cout << "Name: " << decl->name.lexeme;
    
    newline();

    std::cout << "Body:";
    indent();
    newline();
    
    decl->block->accept(this);
    dedent();

    dedent();
}

auto AstPrinter::visit(AssignStatement* stmt) -> void {
    std::cout << "AssignStatement: ";
    
    indent();
    newline();
    std::cout << "LValue: " << stmt->lvalue.lexeme;
    
    newline();
    std::cout << "RValue: ";

    indent();
    newline();
    stmt->rvalue->accept(this);
    dedent();

    dedent();
}

auto AstPrinter::visit(CallStatement* stmt) -> void {
    std::cout << "CallStatement: " << stmt->callee.lexeme;
}

auto AstPrinter::visit(InputStatement* stmt) -> void {
    std::cout << "InputStatement: " << stmt->destination.lexeme;
}

auto AstPrinter::visit(PrintStatement* stmt) -> void {
    std::cout << "PrintStatement: " << stmt->argument.lexeme;
}

auto AstPrinter::visit(BeginStatement* stmt) -> void {
    std::cout << "BeginStatement:";
    
    indent();

    for(const auto& statement : stmt->statements) {
        newline();
        statement->accept(this);
    }

    dedent();
}

auto AstPrinter::visit(IfStatement* stmt) -> void {
    std::cout << "IfStatement:";
    
    indent();
    newline();

    std::cout << "Condition:";

    indent();
    newline();
    stmt->condition->accept(this);
    dedent();

    newline();
    std::cout << "Body: ";
    indent();
    newline();

    stmt->body->accept(this);
    dedent();

    dedent();
}


auto AstPrinter::visit(WhileStatement* stmt) -> void {

    std::cout << "WhileStatement:";
    
    indent();
    newline();

    std::cout << "Condition:";

    indent();
    newline();
    stmt->condition->accept(this);
    dedent();

    newline();
    std::cout << "Body: ";
    indent();
    newline();

    stmt->body->accept(this);
    dedent();

    dedent();
}
    
auto AstPrinter::visit(OddExpression* expr) -> void {    
    
    std::cout << "OddExpression:";
    
    indent();
    newline();

    expr->expr->accept(this);
    dedent();
}

auto AstPrinter::visit(BinaryExpression* expr) -> void {
    std::cout << "BinaryExpression:";
    indent();
    newline();
    
    std::cout << "Operator: " << expr->op.lexeme;
    newline();

    std::cout << "Left:";
    indent();
    newline();
    expr->left->accept(this);
    dedent();

    newline();
    std::cout << "Right:";
    indent();
    newline();
    expr->right->accept(this);
    dedent();

    dedent();
}

auto AstPrinter::visit(UnaryExpression* expr) -> void {
    std::cout << "UnaryExpression:";
    indent();
    newline();
    
    std::cout << "Operator: " << expr->op.lexeme;
    newline();

    std::cout << "Right:";
    indent();
    newline();
    expr->right->accept(this);
    dedent();

    dedent();
}

auto AstPrinter::visit(VariableExpression* expr) -> void {
    std::cout << "VariableExpression: " << expr->name.lexeme;
}

auto AstPrinter::visit(LiteralExpression* expr) -> void {
    std::cout << "LiteralExpression: " << expr->value;
}

}
