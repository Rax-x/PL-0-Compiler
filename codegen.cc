#include "codegen.hpp"
#include "os.hpp"

#include "llvm/ADT/SmallVector.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/LegacyPassManager.h"

#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

#include <string_view>
#include <system_error>

namespace pl0::codegen {

using token::TokenType;

CodeGenerator::CodeGenerator(std::string_view moduleName)
    : m_moduleName(moduleName) {
    m_module = std::make_unique<Module>(moduleName, m_context);

    // printf & scanf functions signature
    FunctionType* ioFunctionType = FunctionType::get(getIntegerType(),
                                                     {PointerType::get(m_builder.getInt8Ty(), 0)}, 
                                                     true);
    // printf & scanf declaration
    Function::Create(ioFunctionType, Function::ExternalLinkage, "printf", m_module.get());
    Function::Create(ioFunctionType, Function::ExternalLinkage, "scanf", m_module.get());

    // main
    FunctionType* funcType = FunctionType::get(getIntegerType(), false);
    Function* function = Function::Create(funcType, Function::ExternalLinkage, "main", m_module.get());
    
    BasicBlock *mainBlock = BasicBlock::Create(m_context, "entry", function);
    m_builder.SetInsertPoint(mainBlock);

    // printf & scanf format strings
    m_builder.CreateGlobalStringPtr("%d\n", "__printf_fmt");
    m_builder.CreateGlobalStringPtr("%d", "__scanf_fmt");

}

auto CodeGenerator::generate(StatementPtr& ast) -> bool {

    codegenStatement(ast);
    endProgram();

    return !verifyModule(*m_module, &errs());
}

auto CodeGenerator::produceObjectFile() -> void {      

    const std::string& targetTriple = llvm::sys::getDefaultTargetTriple();
    
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    std::string error;
    const Target* target = TargetRegistry::lookupTarget(targetTriple, error);

    if(target == nullptr) {
        errs() << error;
        return;
    }

    TargetOptions opt;
    TargetMachine* targetMachine = target->createTargetMachine(targetTriple, "generic", "", opt, Reloc::PIC_);

    m_module->setDataLayout(targetMachine->createDataLayout());
    m_module->setTargetTriple(targetTriple);

    std::error_code streamErrorCode;
    llvm::raw_fd_ostream stream(m_moduleName + ".o", streamErrorCode, sys::fs::OF_None);

    if(streamErrorCode) {
        errs() << "Could not open the file: " << streamErrorCode.message() << '\n';
        return;
    }

    legacy::PassManager pass;
    CodeGenFileType fileType = CodeGenFileType::ObjectFile;

    if (targetMachine->addPassesToEmitFile(pass, stream, nullptr, fileType)) {
        errs() << "TargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*m_module);
    stream.flush();
}

auto CodeGenerator::produceExecutable() -> bool {
    const std::string objectFile = m_moduleName + ".o";

#ifdef __GNUC__
    const char* const compilerName = "g++";
#elif __clang__
    const char* const compilerName = "clang";
#else
    #error Unsupported compiler
#endif
    
    char* const args[] = {
        const_cast<char*>(compilerName), 
        const_cast<char*>(objectFile.c_str()), 
        const_cast<char*>("-o"), 
        const_cast<char*>(m_moduleName.c_str()),
        nullptr,
    };

    return os::spawnProcess(compilerName, args) == 0;
}

auto CodeGenerator::endProgram() -> void {
    m_builder.CreateRet(getIntegerConstant(0));

    if(verifyFunction(*m_builder.GetInsertBlock()->getParent())) {
        error("Compile Error: unable to compile the program.");
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
        std::string name{ident.lexeme};
        Value* value = getIntegerConstant(initializer);

        m_symtable->insert(name, SymbolEntry::constant(value));
    }
}

auto CodeGenerator::visit(VariableDeclarations* decl) -> void {

    const bool areGlobals = !m_symtable->hasParent();
    Function* function = m_builder.GetInsertBlock()->getParent();
    Type* variableType = getIntegerType();

    for(const auto& [_, lexeme, line] : decl->identifiers){
        
        Value* value;
        std::string name{lexeme};

        if(!areGlobals) {

            IRBuilder<> tmpIRBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
            value = tmpIRBuilder.CreateAlloca(variableType, nullptr, name);
            tmpIRBuilder.CreateStore(getIntegerConstant(0), value);
        } else {

            GlobalVariable* global = new GlobalVariable(*m_module, 
                                             variableType, 
                                             false, 
                                             GlobalVariable::ExternalLinkage,
                                             getIntegerConstant(0),
                                             name);

           global->setDSOLocal(true);
           value = global;
        }

        if(!m_symtable->insert(name, SymbolEntry::variable(value))) {
            const char* const kind = areGlobals ? "global" : "local";
            error("[Ln: {}] Compile Error: {} variable '{}' already declared.", line, kind, lexeme);
            return;
        }
    }
}

auto CodeGenerator::visit(ProcedureDeclaration* decl) -> void {

    std::string name{decl->name.lexeme};

    FunctionType* procedureType = FunctionType::get(m_builder.getVoidTy(), false);
    Function* proc = Function::Create(procedureType, Function::ExternalLinkage, name, m_module.get());

    m_symtable->insert(name, SymbolEntry::procedure(proc));

    BasicBlock* prevBlock = m_builder.GetInsertBlock();
    BasicBlock* procedureBlock = BasicBlock::Create(m_context, "entry", proc);

    m_builder.SetInsertPoint(procedureBlock);

    codegenStatement(decl->block);
    m_builder.CreateRetVoid();

    if(verifyFunction(*proc)) {
        error("[Ln: {}] Compile Error: unable to compile '{}' procedure.", decl->name.line, name);
        return;
    }

    m_builder.SetInsertPoint(prevBlock);
}

auto CodeGenerator::visit(AssignStatement* stmt) -> void {
    std::string name{stmt->lvalue.lexeme};

    SymbolEntry* entry = m_symtable->lookup(name);
    if(entry == nullptr) {
        error("[Ln: {}] Compile Error: '{}' undeclared variable.", stmt->lvalue.line, stmt->lvalue.lexeme);
        return;
    }

    if(!entry->isVariable()){
        error("[Ln: {}] Compile Error: can't assign to a constant or a procedure.", stmt->lvalue.line, stmt->lvalue.lexeme);
        return;
    }

    Value* rvalue = codegenExpression(stmt->rvalue);
    m_builder.CreateStore(rvalue, entry->variable());
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

auto CodeGenerator::visit(InputStatement* stmt) -> void {

    std::string name{stmt->destination.lexeme};
    SymbolEntry* entry = m_symtable->lookup(name);

    if(entry == nullptr) {
        error("[Ln: {}] Compile Error: '{}' undeclared variable.", stmt->destination.line, name);
        return;
    }

    if(entry->isProcedure() || entry->isConstant()){
        error("[Ln: {}] Compile Error: can store data only in variables '{}'.", stmt->destination.line, name);
        return;
    }

    SmallVector<Value*, 2> args;

    args.push_back(m_module->getNamedValue("__scanf_fmt"));
    args.push_back(entry->variable());

    m_builder.CreateCall(m_module->getFunction("scanf"), args, "call_scanftmp");
}

auto CodeGenerator::visit(PrintStatement* stmt) -> void {

    SmallVector<Value*, 2> args;

    args.push_back(m_module->getNamedValue("__printf_fmt"));
    args.push_back(codegenExpression(stmt->argument));

    m_builder.CreateCall(m_module->getFunction("printf"), args, "call_printftmp");
}

auto CodeGenerator::visit(BeginStatement* stmt) -> void {
    for(auto& statement : stmt->statements) {
        codegenStatement(statement);
    }
}

auto CodeGenerator::visit(IfStatement* stmt) -> void {

    Value* condition = codegenExpression(stmt->condition);
    
    if(condition == nullptr) {
        error("Compile Error: unable to generate the code for the condition.");
        return;
    }

    Function* currentProcedure = m_builder.GetInsertBlock()->getParent();

    BasicBlock* thenBlock = BasicBlock::Create(m_context, "then", currentProcedure);
    BasicBlock* endBlock = BasicBlock::Create(m_context, "end");

    m_builder.CreateCondBr(condition, thenBlock, endBlock);
    m_builder.SetInsertPoint(thenBlock);

    codegenStatement(stmt->body);
    m_builder.CreateBr(endBlock);
    
    currentProcedure->insert(currentProcedure->end(), endBlock);
    m_builder.SetInsertPoint(endBlock);
}

auto CodeGenerator::visit(WhileStatement* stmt) -> void {

    Function* currentProcedure = m_builder.GetInsertBlock()->getParent();

    BasicBlock* whileBlock = BasicBlock::Create(m_context, "while", currentProcedure);
    BasicBlock* whileBodyBlock = BasicBlock::Create(m_context, "while_body");
    BasicBlock* endBlock = BasicBlock::Create(m_context, "loop_end");

    m_builder.CreateBr(whileBlock);
    m_builder.SetInsertPoint(whileBlock);

    Value* condition = codegenExpression(stmt->condition);
    
    if(condition == nullptr) {
        error("Compile Error: unable to generate the code for the condition.");
        return;
    }

    m_builder.CreateCondBr(condition, whileBodyBlock, endBlock);
    
    currentProcedure->insert(currentProcedure->end(), whileBodyBlock);
    m_builder.SetInsertPoint(whileBodyBlock);

    codegenStatement(stmt->body);
    m_builder.CreateBr(whileBlock);
    
    currentProcedure->insert(currentProcedure->end(), endBlock);
    m_builder.SetInsertPoint(endBlock);
}
    
auto CodeGenerator::visit(OddExpression* expr) -> void {

    Value* left = codegenExpression(expr->expr);
    Value* right = getIntegerConstant(2);
    
    if(left == nullptr || right == nullptr) {
        error("Compile Error: unable to generate the code for the odd expression.");
        return;
    }

    left = m_builder.CreateSRem(left, right, "sremtmp");
    right = getIntegerConstant(0);

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
        error("[Ln: {}] Compile Error: unable to generate the code for this expression.", expr->op.line);
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
        error("[Ln: {}] Compile Error: unable to generate the code for the following expression.", expr->op.line);
        return;
    }

    if(expr->op.type == TokenType::Minus){
        setValue(m_builder.CreateSub(getIntegerConstant(0), right, "unary_subtmp"));
    } else {
        setValue(right);
    }
}

auto CodeGenerator::visit(VariableExpression* expr) -> void {
    
    std::string name{expr->name.lexeme};
    auto entry = m_symtable->lookup(name);

    if(entry == nullptr) {
        error("[Ln: {}] Compile Error: undeclared variable '{}'.", expr->name.line, name);
        return;
    }

    if(entry->isConstant()){
        setValue(entry->constant());
        return;
    }
    
    if(entry->isVariable()){
        setValue(m_builder.CreateLoad(getIntegerType(), entry->variable(), name.c_str()));
        return;
    }

    error("[Ln: {}] Compile Error: functions are not first class objects.", expr->name.line);
}

auto CodeGenerator::visit(LiteralExpression* expr) -> void {
    setValue(getIntegerConstant(expr->value));
}

}



