#ifndef _CODEGEN_HPP_
#define _CODEGEN_HPP_

#include "ast.hpp"
#include "errors_holder_trait.hpp"
#include "symtable.hpp"

#include "llvm/IR/Value.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Constants.h"

#include <memory>
#include <format>
#include <string_view>

namespace pl0::codegen {

using namespace symtable;
using namespace ast;
using namespace error;
using namespace llvm;

class CodeGenerator : public AstVisitor, 
                      public ErrorsHolderTrait {
public:
    CodeGenerator(std::string_view moduleName);

    auto generate(StatementPtr& stmt) -> bool;
    auto produceObjectFile() -> void;

    inline auto dumpLLVM() const -> void {
        m_module->print(outs(), nullptr);
    }

private:

    auto beginScope() -> void;
    auto endScope() -> void;
    auto endProgram() -> void;

    inline auto getIntegerType() -> Type* {
        return m_builder.getInt32Ty();
    }

    inline auto getIntegerConstant(int value) -> Constant* {
        return ConstantInt::getSigned(getIntegerType(), value);
    }

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

    std::string m_moduleName;

    Value* m_value = nullptr;
    LLVMContext m_context;
    IRBuilder<> m_builder = IRBuilder<>(m_context);
    std::unique_ptr<Module> m_module;

    std::shared_ptr<SymbolTable> m_symtable = nullptr; 

    std::vector<std::string> m_errors;
};

}

#endif
