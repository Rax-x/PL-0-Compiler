#include "symtable.hpp"

namespace pl0::symtable {

auto SymbolTable::lookup(const std::string& key) -> SymbolEntry* {

    if(m_symbols.contains(key)) {
        return &m_symbols.at(key);
    }

    if(m_parent == nullptr) return nullptr;

    return m_parent->lookup(key);
}

auto SymbolTable::insert(const std::string& key, SymbolEntry info) -> bool {

    if(m_symbols.contains(key)) {
        return false;
    }

    m_symbols[key] = std::move(info);
    return true;
}

}
