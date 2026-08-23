//  /core/src/ExecutionContext.cpp
#include "synapse/ExecutionContext.hpp"

#include <unordered_set>

namespace Synapse {

    void ExecutionContext::importModule(const Module* mod) {
        if (mod)
            _imports.push_back(mod);
    }

    void ExecutionContext::defineVariable(const char* name, Value val, bool is_const) {
        _local_memory.defineVariable(name, std::move(val), is_const);
    }

    void ExecutionContext::assignVariable(const char* name, Value val) {
        _local_memory.assignVariable(name, std::move(val));
    }

    Value ExecutionContext::getVariable(const char* name) const {
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

    bool ExecutionContext::hasLocalVariable(const char* name) const {
        return _local_memory.hasVariable(name);
    }

    ICallable* ExecutionContext::getFunction(const char* name) const {
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

    void ExecutionContext::enumerateVariables(VariableCallback callback) const {
        std::unordered_set<std::string> seen;

        //  locally
        _local_memory.enumerateVariables([&](const SymbolTable::VariableInfo& info) {
            seen.insert(std::string(info.name));
            callback({std::string(info.name), *info.value, "Local"});
        });

        //for(const auto& import : _imports) {
        for (auto it = _imports.size(); it > 0; --it) {
            auto import = _imports[it - 1];
            std::string import_name = import->getName();

            import->getTable().enumerateVariables([&](const SymbolTable::VariableInfo& info) {
                std::string name(info.name);
                if (seen.find(name) == seen.end()) {
                    seen.insert(name);
                    callback({name, *info.value, import_name});
                }
            });
        }
    }

    void ExecutionContext::enumerateFunctions(FunctionCallback callback) const {
        std::unordered_set<std::string> seen;

        //  locally
        _local_memory.enumerateFunctions([&](const SymbolTable::FunctionInfo& info) {
            seen.insert(std::string(info.name));
            callback({std::string(info.name), info.arity, "Local"});
        });

        //for(const auto& import : _imports) {
        for (auto it = _imports.size(); it > 0; --it) {
            auto import = _imports[it - 1];
            std::string import_name = import->getName();

            import->getTable().enumerateFunctions([&](const SymbolTable::FunctionInfo& info) {
                std::string name(info.name);
                if (seen.find(name) == seen.end()) {
                    seen.insert(name);
                    callback({name, info.arity, import_name});
                }
            });
        }
    }

}   //  namespace   Synapse
