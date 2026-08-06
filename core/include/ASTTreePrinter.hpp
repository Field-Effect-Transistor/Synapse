//  /core/include/ASTTreePrinter.hpp
#pragma once

#include "interface/IVisitor.hpp"
#include "ASTNode.hpp"
#include <string>

namespace Synapse {

    class ASTTreePrinter : public IVisitor {
        std::string _output;
        std::string _prefix;
        
    public:
        ASTTreePrinter() = default;
        ~ASTTreePrinter() = default;

        Value visit(LiteralNode& node) override {
            _output += node._token.lexeme + "\n";
            return Value();
        }

        Value visit(VariableNode& node) override {
            _output += node._token.lexeme + "\n";
            return Value();
        }

        Value visit(BinaryNode& node) override {
            _output += node._token.lexeme + "\n";
            
            std::string saved_prefix = _prefix;
            
            _output += _prefix + "├── ";
            _prefix += "│   ";
            node._left->accept(*this);
            
            _prefix = saved_prefix;
            
            _output += _prefix + "└── ";
            _prefix += "    ";
            node._right->accept(*this);
            
            _prefix = saved_prefix;
            
            return Value();
        }

        Value visit(UnaryNode& node) override {
            _output += node._token.lexeme + "\n";
            
            std::string saved_prefix = _prefix;
            
            _output += _prefix + "└── ";
            _prefix += "    ";
            node._child->accept(*this);
            
            _prefix = saved_prefix;
            
            return Value();
        }

        Value visit(FunctionNode& node) override {
            _output += node._token.lexeme + "\n";
            
            std::string saved_prefix = _prefix;
            
            for (size_t i = 0; i < node._args.size(); ++i) {
                bool is_last = (i == node._args.size() - 1);
                
                _output += _prefix + (is_last ? "└── " : "├── ");
                
                _prefix += (is_last ? "    " : "│   ");
                
                node._args[i]->accept(*this);
                
                _prefix = saved_prefix;
            }
            
            return Value();
        }

        inline const std::string& result() const {
            return _output;
        }

        inline void clear() {
            _output.clear();
            _prefix.clear();
        }

    };  //  class   ASTTreePrinter

}   //  namespace   Synapse
