//  /core/include/synapse/Module.hpp
#pragma once

#include "synapse/SymbolTable.hpp"
#include <string>

namespace Synapse {

    class Module final {
    private:
        std::string _name;
        SymbolTable _table;

    public:
        explicit Module(const char* name) : _name(name) {}

        const std::string& getName() const { return _name; }

        SymbolTable& getTable() { return _table; }
        
        const SymbolTable& getTable() const { return _table; }

    };  //  class   Module

}   //  namespace   Synapse
