#ifndef _SYMTABLE_HPP_
#define _SYMTABLE_HPP_

#include <llvm-18/llvm/IR/Constant.h>
#include <llvm-18/llvm/IR/GlobalVariable.h>
#include <memory>
#include <string>
#include <cstdint>
#include <variant>
#include <unordered_map>

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"

namespace pl0::symtable {

class SymbolEntry {
private:
    SymbolEntry(std::string name, llvm::Value* value, std::uint32_t line)
        : m_name(std::move(name)), m_data(value), m_line(line) {}

    SymbolEntry(std::string name, llvm::AllocaInst* value, std::uint32_t line, bool isGlobal = false)
        : m_name(std::move(name)), m_data(value), m_line(line), m_isGlobal(isGlobal) {}

    SymbolEntry(std::string name, llvm::Function* value, std::uint32_t line)
        : m_name(std::move(name)), m_data(value), m_line(line) {}

public:
    SymbolEntry() = default;

    static auto constant(std::string name, llvm::Value* value, std::uint32_t line) -> SymbolEntry {
        return SymbolEntry(std::move(name), value, line);
    }

    static auto variable(std::string name, llvm::AllocaInst* value, std::uint32_t line, bool gloabl = false) -> SymbolEntry {
        return SymbolEntry(std::move(name), value, line, gloabl);
    }

    static auto procedure(std::string name, llvm::Function* value, std::uint32_t line) -> SymbolEntry {
        return SymbolEntry(std::move(name), value, line);
    }

    constexpr auto isProcedure() const -> bool {
        return std::holds_alternative<llvm::Function*>(m_data);
    }

    constexpr auto isConstant() const -> bool {
        return std::holds_alternative<llvm::Value*>(m_data);
    }

    constexpr auto isVariable() const -> bool {
        return std::holds_alternative<llvm::AllocaInst*>(m_data);
    } 

    constexpr auto isGlobal() const -> bool {
        return m_isGlobal;
    }

    constexpr auto procedure() const -> llvm::Function* {
        return std::get<llvm::Function*>(m_data);
    }

    constexpr auto constant() -> llvm::Value* {
        return std::get<llvm::Value*>(m_data);
    }

    constexpr auto variable() -> llvm::AllocaInst* {
        return std::get<llvm::AllocaInst*>(m_data);
    } 

    inline auto name() const -> const std::string& {
        return m_name;
    }

    constexpr auto line() const -> std::uint32_t {
        return m_line;
    }
    
private:
    std::string m_name;
    std::variant<llvm::Value*, llvm::Function*, llvm::AllocaInst*>  m_data;
    std::uint32_t m_line;
    bool m_isGlobal;
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
