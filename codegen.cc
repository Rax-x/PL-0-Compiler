#include "codegen.hpp"
#include "ast.hpp"
#include "symtable.hpp"
#include <iostream>
#include <llvm-18/llvm/ADT/ArrayRef.h>
#include <llvm-18/llvm/IR/BasicBlock.h>
#include <llvm-18/llvm/IR/CallingConv.h>
#include <llvm-18/llvm/IR/Constant.h>
#include <llvm-18/llvm/IR/Constants.h>
#include <llvm-18/llvm/IR/GlobalValue.h>
#include <llvm-18/llvm/IR/GlobalVariable.h>
#include <llvm-18/llvm/IR/Instructions.h>

namespace pl0::codegen {

using token::TokenType;

CodeGenerator::CodeGenerator() {

    m_module = std::make_unique<Module>("pl0 program", m_context);

    FunctionType* funcType = FunctionType::get(Type::getVoidTy(m_context), false);
    Function* function = Function::Create(funcType, Function::ExternalLinkage, "__main", m_module.get());
    
    BasicBlock *mainBlock = BasicBlock::Create(m_context, "main_block", function);
    m_builder.SetInsertPoint(mainBlock);
}

CodeGenerator::~CodeGenerator(){
    verifyModule(*m_module, &errs());
}

auto CodeGenerator::endProgram() -> void {
    m_builder.CreateRetVoid();
    verifyFunction(*m_builder.GetInsertBlock()->getParent());
}

auto CodeGenerator::dump(StatementPtr& stmt) -> void {
    codegenStatement(stmt);
    endProgram();

    if(hadError()){
        for(const auto& error : errors()){
            std::cout << error << '\n';
        }
    } else {
        m_module->dump();  
    }
}

auto CodeGenerator::beginScope() -> void {
    m_symtable = std::make_shared<SymbolTable>(m_symtable);
}

auto CodeGenerator::endScope() -> void {
    if(!m_symtable->hasParent()) return;
    m_symtable = m_symtable->parent();
}

auto CodeGenerator::visit(Block* block) -> void {

    beginScope();

    codegenStatement(block->constantsDeclaration);
    codegenStatement(block->variablesDeclaration);

    for(auto& procedure : block->procedureDeclarations){
        codegenStatement(procedure);
    }
    
    codegenStatement(block->statement);

    endScope();
}

auto CodeGenerator::visit(ConstDeclarations* decl) -> void {
    
    for(const auto& [ident, initializer] : decl->declarations) {
        auto name = std::string(ident.lexeme);
        auto value = ConstantInt::getSigned(Type::getInt32Ty(m_context), initializer);

        m_symtable->insert(name, SymbolEntry::constant(name, value, ident.line));
    }
}

auto CodeGenerator::visit(VariableDeclarations* decl) -> void {

    auto areGlobals = !m_symtable->hasParent();
    auto function = m_builder.GetInsertBlock()->getParent();
    auto variableType = Type::getInt32Ty(m_context);

    for(const auto& [_, lexeme, line] : decl->identifiers){
        
        AllocaInst* allocaInst = nullptr;
        auto name = std::string(lexeme);

        if(!areGlobals) {
            IRBuilder<> tmpIRBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
            allocaInst = tmpIRBuilder.CreateAlloca(variableType, nullptr, name);
        } else {
            m_module->getOrInsertGlobal(name, variableType);
        }

        m_symtable->insert(name, SymbolEntry::variable(name, allocaInst, line, areGlobals));
    }
}

auto CodeGenerator::visit(ProcedureDeclaration* decl) -> void {

    auto name = std::string(decl->name.lexeme);
    FunctionType* funcType = FunctionType::get(Type::getVoidTy(m_context), false);
    Function* function = Function::Create(funcType, Function::ExternalLinkage, name, m_module.get());

    m_symtable->insert(name, SymbolEntry::procedure(name, function, decl->name.line));

    BasicBlock* prevBlock = m_builder.GetInsertBlock();

    BasicBlock* bblock = BasicBlock::Create(m_context, "entry_"+name, function);
    m_builder.SetInsertPoint(bblock);

    codegenStatement(decl->block);
    m_builder.CreateRetVoid();


    verifyFunction(*function);

    m_builder.SetInsertPoint(prevBlock);
}

auto CodeGenerator::visit(AssignStatement* stmt) -> void {
    auto name = std::string(stmt->lvalue.lexeme);

    auto entry = m_symtable->lookup(name);
    if(entry == nullptr) {
        error("[Ln: {}] Compile Error: '{}' undeclared variable.", stmt->lvalue.line, stmt->lvalue.lexeme);
        return;
    }

    if(!entry->isVariable()){
        error("[Ln: {}] Compile Error: can't assign to a constant or a procedure.", stmt->lvalue.line, stmt->lvalue.lexeme);
        return;
    }


    Value* rvalue = codegenExpression(stmt->rvalue);

    Value* ptr;
    if(entry->isGlobal()) {
        ptr = m_module->getNamedGlobal(name);
    } else {
        ptr =  entry->variable();
    }
    
    m_builder.CreateStore(rvalue, ptr); 
}

auto CodeGenerator::visit(CallStatement* stmt) -> void {

    SymbolEntry* entry = m_symtable->lookup(std::string(stmt->callee.lexeme));

    if(entry == nullptr) {
        error("[Ln: {}] Compile Error: '{}' undeclared procedure.", stmt->callee.line, stmt->callee.lexeme);
        return;
    }

    if(!entry->isProcedure()) {
        error("[Ln: {}] Compile Error: '{}' is not callable.", stmt->callee.line, stmt->callee.lexeme);
        return;
    }

    m_builder.CreateCall(entry->procedure(), std::nullopt);
}

auto CodeGenerator::visit(InputStatement* stmt) -> void {}

auto CodeGenerator::visit(PrintStatement* stmt) -> void {
    // TODO: call printf
    // https://github.com/thomaslee/llvm-demo/blob/master/main.cc
}

auto CodeGenerator::visit(BeginStatement* stmt) -> void {
    for(auto& statement : stmt->statements) {
        codegenStatement(statement);
    }
}

auto CodeGenerator::visit(IfStatement* stmt) -> void {

    Value* condValue = codegenExpression(stmt->condition);
    
    if(condValue == nullptr) {
        error("Compile Error: unable to generate the code for the condition.");
        return;
    }

    Function* function = m_builder.GetInsertBlock()->getParent();

    BasicBlock* thenBlock = BasicBlock::Create(m_context, "then", function);
    BasicBlock* endBlock = BasicBlock::Create(m_context, "end");

    m_builder.CreateCondBr(condValue, thenBlock, endBlock);
    
    m_builder.SetInsertPoint(thenBlock);
    codegenStatement(stmt->body);
    
    m_builder.CreateBr(endBlock);
    
    function->insert(function->end(), endBlock);
    m_builder.SetInsertPoint(endBlock);
}

auto CodeGenerator::visit(WhileStatement* stmt) -> void {

    Function* function = m_builder.GetInsertBlock()->getParent();

    BasicBlock* whileBlock = BasicBlock::Create(m_context, "while", function);
    BasicBlock* whileBodyBlock = BasicBlock::Create(m_context, "while_body");
    BasicBlock* endBlock = BasicBlock::Create(m_context, "loop_end");

    m_builder.CreateBr(whileBlock);
    m_builder.SetInsertPoint(whileBlock);

    Value* condValue = codegenExpression(stmt->condition);
    
    if(condValue == nullptr) {
        error("Compile Error: unable to generate the code for the condition.");
        return;
    }

    m_builder.CreateCondBr(condValue, whileBodyBlock, endBlock);
    
    function->insert(function->end(), whileBodyBlock);
    m_builder.SetInsertPoint(whileBodyBlock);
    codegenStatement(stmt->body);
    
    m_builder.CreateBr(whileBlock);
    
    function->insert(function->end(), endBlock);
    m_builder.SetInsertPoint(endBlock);
}
    
auto CodeGenerator::visit(OddExpression* expr) -> void {

    auto type = Type::getInt32Ty(m_context);

    Value* left = codegenExpression(expr->expr);
    Value* right = ConstantInt::getSigned(type, 2);
    
    if(left == nullptr || right == nullptr) {
        error("Compile Error: unable to generate the code for the odd expression.");
        return;
    }

    left = m_builder.CreateSRem(left, right, "sremtmp");
    right = ConstantInt::getSigned(type, 0);

    if(left == nullptr || right == nullptr) {
        error("Compile Error: unable to generate the code for the odd expression.");
        return;
    }

    setValue(m_builder.CreateICmpNE(left, right, "ne_icmptmp"));
}

auto CodeGenerator::visit(BinaryExpression* expr) -> void {
    Value* left = codegenExpression(expr->left);
    Value* right = codegenExpression(expr->right);

    if(left == nullptr || right == nullptr) {
        error("[Ln: {}] Compile Error: unable to generate the code for the following expression.", expr->op.line);
        return;
    }

    switch(expr->op.type){
        case TokenType::Plus:
            setValue(m_builder.CreateAdd(left, right, "addtmp"));
            break;
        case TokenType::Minus:
            setValue(m_builder.CreateSub(left, right, "subtmp"));
            break;
        case TokenType::Star:
            setValue(m_builder.CreateMul(left, right, "multmp"));
            break;
        case TokenType::Slash:
            setValue(m_builder.CreateSDiv(left, right, "divtmp"));
            break;
        case TokenType::Greater:
            setValue(m_builder.CreateICmpSGT(left, right, "sgt_icmptmp"));
            break;
        case TokenType::GreaterEqual:
            setValue(m_builder.CreateICmpSGE(left, right, "sge_icmptmp"));
            break;
        case TokenType::Less:
            setValue(m_builder.CreateICmpSLT(left, right, "slt_icmptmp"));
            break;
        case TokenType::LessEqual:
            setValue(m_builder.CreateICmpSLE(left, right, "sle_icmptmp"));
            break; 
        case TokenType::Equal:
            setValue(m_builder.CreateICmpEQ(left, right, "eq_icmptmp"));
            break;
        case TokenType::NotEqual:
            setValue(m_builder.CreateICmpNE(left, right, "ne_icmptmp"));
            break;
        default:
            error("[Ln: {}] Compile Error: '{}' is an invalid binary operator.", expr->op.line, expr->op.lexeme);
            break;
    }
}

auto CodeGenerator::visit(UnaryExpression* expr) -> void {

    Value* right = codegenExpression(expr->right);

    if(right == nullptr) {
        error("[Ln: {} ] Compile Error: unable to generate the code for the following expression.", expr->op.line);
        return;
    }

    if(expr->op.type == TokenType::Minus){
        Value* left = ConstantInt::getSigned(Type::getInt32Ty(m_context), 0);
        setValue(m_builder.CreateSub(left, right, "unary_subtmp"));
    } else {
        setValue(right);
    }
}


auto CodeGenerator::visit(VariableExpression* expr) -> void {
    
    std::string name(expr->name.lexeme);
    auto entry = m_symtable->lookup(name);

    if(entry == nullptr) {
        error("Compile Error: undeclared variable '{}'.", name);
        return;
    }

    if(entry->isConstant()){
        setValue(entry->constant());
    } else if(entry->isVariable()){

        Value* value;
        if(!entry->isGlobal()){
            auto allocaInst = entry->variable();
            value = m_builder.CreateLoad(allocaInst->getAllocatedType(), 
                                         allocaInst,
                                         name.c_str());
        } else {
            GlobalVariable* variable = m_module->getNamedGlobal(name);
            value = m_builder.CreateLoad(variable->getValueType(), 
                                 variable,
                                 name.c_str());
        }

        setValue(value);
    } else {
        error("Compile Error: function aren't first class objects.");
        return;
    }
}

auto CodeGenerator::visit(LiteralExpression* expr) -> void {
    auto type = Type::getInt32Ty(m_context);
    setValue(ConstantInt::getSigned(type, expr->value));
}

}



