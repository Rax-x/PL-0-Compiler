#ifndef _SYMTABLE_HPP_
#define _SYMTABLE_HPP_

#include <memory>
#include <string>
#include <variant>
#include <unordered_map>

#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"

namespace pl0::symtable {


class SymbolEntry {
private:
    SymbolEntry(llvm::Value* value, bool isConstant)
        : m_data(value), m_isConstant(isConstant) {}

    SymbolEntry(llvm::Function* value)
        : m_data(value), m_isConstant(false) {}

public:
    SymbolEntry() = default;

    static inline auto constant(llvm::Value* value) -> SymbolEntry {
        return SymbolEntry(value, true);
    }

    static inline auto variable(llvm::Value* value) -> SymbolEntry {
        return SymbolEntry(value, false);
    }

    static inline auto procedure(llvm::Function* value) -> SymbolEntry {
        return SymbolEntry(value);
    }

    constexpr auto isProcedure() const -> bool {
        return std::holds_alternative<llvm::Function*>(m_data);
    }

    constexpr auto isConstant() const -> bool {
        return m_isConstant;
    }

    constexpr auto isVariable() const -> bool {
        return !isConstant() && !isProcedure();
    }

    constexpr auto procedure() const -> llvm::Function* {
        return std::get<llvm::Function*>(m_data);
    }

    constexpr auto constant() -> llvm::Value* {
        return std::get<llvm::Value*>(m_data);
    }

    constexpr auto variable() -> llvm::Value* {
        return std::get<llvm::Value*>(m_data);
    } 

    
private:
    std::variant<llvm::Value*, llvm::Function*>  m_data;
    bool m_isConstant;
};

class SymbolTable : public std::enable_shared_from_this<SymbolTable> {
public:

    SymbolTable(std::shared_ptr<SymbolTable> parent = nullptr)
        : m_parent(std::move(parent)) {}

    auto lookup(const std::string& key) -> SymbolEntry*;
    auto insert(const std::string& key, SymbolEntry info) -> bool;

    inline auto hasParent() -> bool {
        return m_parent != nullptr;
    }

    inline auto parent() -> std::shared_ptr<SymbolTable> {
        return m_parent;
    }
    
private:
    std::shared_ptr<SymbolTable> m_parent;
    std::unordered_map<std::string, SymbolEntry> m_symbols;    
};

}

#endif
