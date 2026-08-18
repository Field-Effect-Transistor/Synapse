//  /plugin/PrinterLib/src/ASTPrinter.cpp
#include "ASTPrinter.hpp"

#include "StringValue.hpp"

#include <string>
#include <cstddef>

namespace PrinterLib {

    struct ASTPrinter::Impl {
        std::string _output;
        size_t      _depth = 0;

        Value checkNreturn() {
            if (_depth == 0) {
                Value res(new StringValue(std::move(_output)));
                _output.clear();
                return res;
            }

            return Value();
        }

    };  //  struct  ASTPrinter::Impl

    ASTPrinter::ASTPrinter() : _impl(new Impl) {};
    ASTPrinter::~ASTPrinter() = default;
    void ASTPrinter::destroy() { delete this; }

    Value ASTPrinter::visit(LiteralNode& node) {
        _impl->_output += node._token.lexeme;
        return _impl->checkNreturn();
    }

    Value ASTPrinter::visit(VariableNode& node) {
        _impl->_output += node._token.lexeme;
        return _impl->checkNreturn();
    }

    Value ASTPrinter::visit(BinaryNode& node) {
        ++_impl->_depth;

         _impl->_output += "( ";
         _impl->_output += node._token.lexeme;
         _impl->_output += " ";
        node._left->accept(*this);
         _impl->_output += " ";
        node._right->accept(*this);
         _impl->_output += " )";

        --_impl->_depth;
        return _impl->checkNreturn();
    }

    Value ASTPrinter::visit(UnaryNode& node) {
        ++_impl->_depth;

         _impl->_output += "( ";
         _impl->_output += node._token.lexeme;
         _impl->_output += " ";
        node._child->accept(*this);
         _impl->_output += " )";

        --_impl->_depth;
        return _impl->checkNreturn();
    }

    Value ASTPrinter::visit(FunctionNode& node) {
        ++_impl->_depth;

         _impl->_output += "( ";
         _impl->_output += node._token.lexeme;
         _impl->_output += " ";
        for (auto& arg : node._args) {
            arg->accept(*this);
             _impl->_output += " ";
        }
         _impl->_output += ")";

        --_impl->_depth;
        return _impl->checkNreturn();
    }

}   //  namespace   PrinterLib
