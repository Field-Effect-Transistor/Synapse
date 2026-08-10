//  /core/src/Context.cpp
#include "Context.hpp"

#include <unordered_map>

namespace Synapse {

    struct Context::Impl {
        std::unordered_map<std::string, Value>          _symbols;
        std::unordered_map<std::string, CallablePtr>    _functions;
    };  //  struct  Context::Impl

    Context::Context(Context* parent) : _impl(new Impl), _parent(parent) {}
    Context::~Context() = default;

    Context::Context(Context&&) noexcept = default;
    Context& Context::operator=(Context&&) noexcept = default;

    void Context::defineVariable(const char* name, Value val, bool is_const) {
        std::string key(name);
        
        if(_impl->_symbols.count(key)) {
            throw std::runtime_error("Variable '" + key + "' is already defined in this scope!");
        }
        _impl->_symbols[key] = std::move(val.set_is_const(is_const));
    }

    void Context::assignVariable(const char* name, Value val) {
        std::string key(name);

        if(auto it = _impl->_symbols.find(key); it == _impl->_symbols.end()) {
            throw std::runtime_error("Variable '" + key + "' is not defined in this scope!");
        } else {
            if(it->second.is_constant()) {
                throw std::runtime_error("Variable '" + key + "' is const in this scope!");
            }
            it->second = std::move(val);
        }
    }

    Value Context::getVariable(const char* name) const {
        std::string key(name);
        const Context* curr = this;

        while (curr != nullptr) {
            auto it = curr->_impl->_symbols.find(key);
            
            if (it != curr->_impl->_symbols.end()) {
                return it->second;
            }
            
            curr = curr->_parent;
        }

        throw std::runtime_error("Variable '" + key + "' is not defined!");
    }

    void Context::defineFunction(const char* name, CallablePtr function) {
        std::string key(name);
        
        if (!function) {
            throw std::runtime_error("Cannot define a null function '" + key + "'!");
        }
        
        if (_impl->_functions.count(key)) {
            throw std::runtime_error("Function '" + key + "' is already defined in this scope!");
        }

        _impl->_functions[key] = std::move(function);
    }

    ICallable* Context::getFunction(const char* name) const {
        std::string key(name);
        const Context* curr = this;

        while (curr != nullptr) {
            auto it = curr->_impl->_functions.find(key);
            
            if (it != curr->_impl->_functions.end()) {
                return it->second.get();
            }
            
            curr = curr->_parent;
        }

        throw std::runtime_error("Function '" + key + "' is not defined!");
    }


}   //  namespace   Synapse
