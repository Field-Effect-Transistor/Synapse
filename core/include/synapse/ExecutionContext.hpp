//  /core/include/synapse/ExecutionContext.hpp
#pragma once

#include "synapse/SymbolTable.hpp"
#include "synapse/Module.hpp"
#include "Vector.hpp"

namespace Synapse {

    class ExecutionContext final {
    private:
        SymbolTable             _local_memory;
        Vector<const Module*>   _imports;

    public:
        ExecutionContext() = default;

        void importModule(const Module* mod) {
            if (mod) _imports.push_back(mod);
        }

        void defineVariable(const char* name, Value val, bool is_const = false) {
            _local_memory.defineVariable(name, std::move(val), is_const);
        }

        void assignVariable(const char* name, Value val) {
            _local_memory.assignVariable(name, std::move(val));
        }

        Value getVariable(const char* name) const {
            if (_local_memory.hasVariable(name)) {
                return _local_memory.getVariable(name);
            }

            for (auto it = _imports.size(); it > 0; --it) {
                const Module* mod = _imports[it - 1];
                if (mod->getTable().hasVariable(name)) {
                    return mod->getTable().getVariable(name);
                }
            }

            throw std::runtime_error(std::string("Variable '") + name + "' is not defined!");
        }

        bool hasLocalVariable(const char* name) const {
            return _local_memory.hasVariable(name);
        }

        ICallable* getFunction(const char* name) const {
            if (_local_memory.hasFunction(name)) {
                return _local_memory.getFunction(name);
            }

            for (auto it = _imports.size(); it > 0; --it) {
                const Module* mod = _imports[it - 1];
                if (mod->getTable().hasFunction(name)) {
                    return mod->getTable().getFunction(name);
                }
            }

            throw std::runtime_error(std::string("Function '") + name + "' is not defined!");
        }

    };  //  class   ExecutionContext

}   //  namespace   Synapse
