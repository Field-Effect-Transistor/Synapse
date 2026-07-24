//  /core/include/ASTPrinter.hpp
#pragma once

#include "interface/IVisitor.hpp"
#include "ASTNode.hpp"

namespace Hermes {

    class ASTPrinter : public IVisitor {
        std::string _output;
        
    public:
        ASTPrinter() = default;
        ~ASTPrinter() = default;


        Value visit(LeafNode& node) override {
            _output += node._value.to_str();
            return Value();
        }

        Value visit(VariableNode& node) override {
            _output += node._token.lexeme;
            return Value();
        }

        Value visit(BinaryNode& node) override {
            _output += "( ";
            _output += node._token.lexeme;
            _output += " ";
            node._left->accept(*this);
            _output += " ";
            node._right->accept(*this);
            _output += " )";
            return Value();
        }

        Value visit(UnaryNode& node) override {
            _output += "( ";
            _output += node._token.lexeme;
            _output += " ";
            node._child->accept(*this);
            _output += " )";
            return Value();
        }

        Value visit(FunctionNode& node) override {
            _output += "( ";
            _output += node._token.lexeme;
            _output += " ";
            for (auto& arg : node._args) {
                arg->accept(*this);
                _output += " ";
            }
            _output += ")";
            return Value();
        }

        inline const std::string& result() const {
            return _output;
        }

        inline void clear() {
            _output.clear();
        }

    };  //  class   ASTPrinter

}   //  namespace   Hermes
