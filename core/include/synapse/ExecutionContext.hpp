//  /core/include/synapse/ExecutionContext.hpp
#pragma once

#include "synapse/SymbolTable.hpp"
#include "synapse/Module.hpp"

#include <functional>

#include "Vector.hpp"

namespace Synapse {

    class ExecutionContext final {
    public:
        struct VariableMeta {
            std::string name;
            Value       value;
            std::string source;
        };  //  struct  VariableMeta

        struct FunctionMeta {
            std::string name;
            size_t      arity;
            std::string source;
        };  //  struct  FunctionMeta

        using VariableCallback = std::function<void(const VariableMeta&)>;
        using FunctionCallback = std::function<void(const FunctionMeta&)>;

    private:
        SymbolTable             _local_memory;
        Vector<const Module*>   _imports;

    public:
        ExecutionContext() = default;

        void importModule(const Module* mod);

        void defineVariable(const char* name, Value val, bool is_const = false);
        void assignVariable(const char* name, Value val);
        Value getVariable(const char* name) const;
        bool hasLocalVariable(const char* name) const;

        ICallable* getFunction(const char* name) const;

        void enumerateVariables(VariableCallback) const;
        void enumerateFunctions(FunctionCallback) const;

    };  //  class   ExecutionContext

}   //  namespace   Synapse
