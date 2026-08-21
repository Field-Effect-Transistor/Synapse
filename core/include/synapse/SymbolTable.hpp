//  /core/include/synapse/SymbolTable.hpp
#pragma once

#include "synapse/Value.hpp"
#include "synapse/interface/ICallable.hpp"

#include "UniquePtr.hpp"

namespace Synapse {

    class   SymbolTable {

        struct Impl;
        UniquePtr<Impl> _impl;

    public:
        SymbolTable();
        ~SymbolTable();

        SymbolTable(const SymbolTable&) = delete;
        SymbolTable& operator=(const SymbolTable&) = delete;

        SymbolTable(SymbolTable&&) noexcept;
        SymbolTable& operator=(SymbolTable&&) noexcept;

        void defineVariable(const char* name, Value val, bool is_const = false);
        void assignVariable(const char* name, Value val);
        Value getVariable(const char* name) const;

        void defineFunction(const char* name, ICallable::Ptr function);
        ICallable* getFunction(const char* name) const;

        bool hasVariable(const char* name) const;
        bool hasFunction(const char* name) const;
        

    };  //  class   SymbolTable

}   //  namespace   Synapse
