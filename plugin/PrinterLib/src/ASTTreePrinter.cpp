//  /plugin/PrinterLib/src/ASTTreePrinter.cpp
#include "ASTTreePrinter.hpp"

#include "StringValue.hpp"

#include <string>
#include <cstddef>

namespace PrinterLib {

    struct ASTTreePrinter::Impl {
        std::string _output;
        std::string _prefix;
        size_t      _depth = 0;

        Value checkNreturn() {
            if (_depth == 0) {
                Value res(new StringValue(std::move(_output)));
                _output.clear();
                _prefix.clear();
                return res;
            }

            return Value();
        }
    };  //  ASTTreePrinter::Impl

    ASTTreePrinter::ASTTreePrinter() : _impl(new Impl) {};
    ASTTreePrinter::~ASTTreePrinter() = default;
    void ASTTreePrinter::destroy() { delete this; }

    Value ASTTreePrinter::visit(LiteralNode& node) {
        _impl->_output += node._token.lexeme + "\n";
        return _impl->checkNreturn();
    }

    Value ASTTreePrinter::visit(VariableNode& node) {
        _impl->_output += node._token.lexeme + "\n";
        return _impl->checkNreturn();
    }

    Value ASTTreePrinter::visit(BinaryNode& node) {
        ++_impl->_depth;

        _impl->_output += node._token.lexeme + "\n";
        
        std::string saved_prefix = _impl->_prefix;
        
        _impl->_output += _impl->_prefix + "├── ";
        _impl->_prefix += "│   ";
        node._left->accept(*this);
        
        _impl->_prefix = saved_prefix;
        
        _impl->_output += _impl->_prefix + "└── ";
        _impl->_prefix += "    ";
        node._right->accept(*this);
        
        _impl->_prefix = saved_prefix;
        
        --_impl->_depth;
        return _impl->checkNreturn();
    }

    Value ASTTreePrinter::visit(UnaryNode& node) {
        ++_impl->_depth;

        _impl->_output += node._token.lexeme + "\n";
        
        std::string saved_prefix = _impl->_prefix;
        
        _impl->_output += _impl->_prefix + "└── ";
        _impl->_prefix += "    ";
        node._child->accept(*this);
        
        _impl->_prefix = saved_prefix;
        
        --_impl->_depth;
        return _impl->checkNreturn();
    }

    Value ASTTreePrinter::visit(FunctionNode& node) {
        ++_impl->_depth;

        _impl->_output += node._token.lexeme + "\n";
        
        std::string saved_prefix = _impl->_prefix;
        
        for (size_t i = 0; i < node._args.size(); ++i) {
            bool is_last = (i == node._args.size() - 1);
            
            _impl->_output += _impl->_prefix + (is_last ? "└── " : "├── ");
            
            _impl->_prefix += (is_last ? "    " : "│   ");
            
            node._args[i]->accept(*this);
            
            _impl->_prefix = saved_prefix;
        }
        
        --_impl->_depth;
        return _impl->checkNreturn();
    }

}   //  namespace   PrinterLib
