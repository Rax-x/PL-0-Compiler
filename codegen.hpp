#ifndef _CODEGEN_HPP_
#define _CODEGEN_HPP_

#include "ast.hpp"
#include "errors_holder_trait.hpp"
#include "symtable.hpp"

#include "llvm/IR/Value.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"

#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

#include <memory>
#include <format>

namespace pl0::codegen {

using namespace symtable;
using namespace ast;
using namespace error;
using namespace llvm;

class CodeGenerator : public AstVisitor, ErrorsHolderTrait {
public:
    CodeGenerator();
    ~CodeGenerator();

    auto generateObjectFile(StatementPtr& stmt, const char* filename) -> void;
    auto dump() -> void;

private:

    auto beginScope() -> void;
    auto endScope() -> void;
    auto endProgram() -> void;

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

    template<typename... Args>
    inline auto error(std::string_view fmt, Args&&... args) -> void {
        pushError(std::vformat(fmt, std::make_format_args(args...)));
        setValue(nullptr);
    }

    inline auto codegenStatement(StatementPtr& stmt) -> void {
        if(stmt == nullptr) return;

        stmt->accept(this);
        setValue(nullptr);
    }

    inline auto codegenExpression(ExpressionPtr& expr) -> llvm::Value* {
        expr->accept(this);
        return getValue();
    }

    constexpr auto setValue(llvm::Value* value) -> void {
        m_value = value;
    }

    constexpr auto getValue() -> llvm::Value* {
        return m_value;
    }

private:

    Value* m_value = nullptr;
    LLVMContext m_context;
    IRBuilder<> m_builder = IRBuilder<>(m_context);
    std::unique_ptr<Module> m_module;

    std::shared_ptr<SymbolTable> m_symtable = nullptr; 

    std::vector<std::string> m_errors;
};

}

#endif