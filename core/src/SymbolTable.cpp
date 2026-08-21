//  /core/src/SymbolTable.cpp
#include "synapse/SymbolTable.hpp"

#include <unordered_map>
#include <string>
#include <stdexcept>

namespace Synapse {

    struct SymbolTable::Impl {
        std::unordered_map<std::string, Value>          _variables;
        std::unordered_map<std::string, ICallable::Ptr> _functions;
    };  //  struct  SymbolTable::Impl

    SymbolTable::SymbolTable() : _impl(new Impl()) {}
    SymbolTable::~SymbolTable() = default;

    SymbolTable::SymbolTable(SymbolTable&&) noexcept = default;
    SymbolTable& SymbolTable::operator=(SymbolTable&&) noexcept = default;

    void SymbolTable::defineVariable(const char* name, Value val, bool is_const) {
        std::string key(name);
        if (_impl->_variables.count(key)) {
            throw std::runtime_error("Variable '" + key + "' is already defined!");
        }
        _impl->_variables[key] = std::move(val.set_is_const(is_const));
    }

    void SymbolTable::assignVariable(const char* name, Value val) {
        std::string key(name);
        auto it = _impl->_variables.find(key);
        if (it == _impl->_variables.end()) {
            throw std::runtime_error("Cannot assign to unknown variable '" + key + "'");
        }
        if (it->second.is_constant()) {
            throw std::runtime_error("Cannot reassign constant variable '" + key + "'");
        }
        it->second = std::move(val);
    }

    bool SymbolTable::hasVariable(const char* name) const {
        return _impl->_variables.count(name) > 0;
    }

    Value SymbolTable::getVariable(const char* name) const {
        std::string key(name);
        auto it = _impl->_variables.find(key);
        if (it != _impl->_variables.end()) {
            return it->second;
        }
        throw std::runtime_error("Variable '" + key + "' is not defined!");
    }

    void SymbolTable::defineFunction(const char* name, ICallable::Ptr function) {
        std::string key(name);
        if (!function) throw std::runtime_error("Cannot define a null function!");
        if (_impl->_functions.count(key)) {
            throw std::runtime_error("Function '" + key + "' is already defined!");
        }
        _impl->_functions[key] = std::move(function);
    }

    bool SymbolTable::hasFunction(const char* name) const {
        return _impl->_functions.count(name) > 0;
    }

    ICallable* SymbolTable::getFunction(const char* name) const {
        std::string key(name);
        auto it = _impl->_functions.find(key);
        if (it != _impl->_functions.end()) {
            return it->second.get();
        }
        throw std::runtime_error("Function '" + key + "' is not defined!");
    }

}   //  namespace   Synapse
