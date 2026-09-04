//  /core/src/MathEvaluator.cpp
#include "internal/MathEvaluator.hpp"

#include "synapse/Exceptions.hpp"
#include "synapse/ExecutionContext.hpp"

#include <string>
#include <cmath>

namespace Synapse::Internal {

    void MathEvaluator::destroy() {
        delete this;
    }

    void MathEvaluator::setContext(ExecutionContext* ctx) {
        _context = ctx;
    }

    Value MathEvaluator::visit(VariableNode& node) {
        if (!_context) throw RuntimeError("Evaluation ExecutionContext is null!");
        
        return _context->getVariable(node._token.lexeme.c_str());
    }

    Value MathEvaluator::visit(LiteralNode& node) {
        return Value(node._token.value);
    }

    Value MathEvaluator::visit(UnaryNode& node) {
        TokenType type = node._token.type;

        Value child_val = node._child->accept(*this);
        double value = child_val.getNumber();

        switch (type) {
            case StandardToken::PERCENT: 
                value = value / 100.0; 
                break;
            case StandardToken::SUB:    
                value = -value; 
                break;
            case StandardToken::ADD:    
                break;
            default:
                throw RuntimeError("Unsupported unary operator: '" + node._token.lexeme + "'");
        }

        return Value(value);
    }

    Value MathEvaluator::visit(BinaryNode& node) {
        TokenType type = node._token.type;

        if (type == StandardToken::ASSIGN) {
            Value right = node._right->accept(*this);
            
            VariableNode* var_node = static_cast<VariableNode*>(node._left.get());
            std::string var_name = var_node->_token.lexeme;

            if (_context->hasLocalVariable(var_name.c_str())) {
                _context->assignVariable(var_name.c_str(), Value(right));
            } else {
                _context->defineVariable(var_name.c_str(), Value(right), false); 
            }
            
            // Return right value to support chained assignments (a = b = c)
            return right;
        }

        Value left = node._left->accept(*this);
        Value right = node._right->accept(*this);

        double l_val = left.getNumber();
        double r_val = right.getNumber();

        switch(type) {
            case StandardToken::ADD: 
                return Value(l_val + r_val);
                
            case StandardToken::SUB: 
                return Value(l_val - r_val);
                
            case StandardToken::MUL: 
                return Value(l_val * r_val);
                
            case StandardToken::DIV: 
                if (r_val == 0.0) {
                    throw RuntimeError(
                        "Division by zero at [Row: " + std::to_string(node._token.row) + 
                        ", Col: " + std::to_string(node._token.column) + "]."
                    );
                }
                return Value(l_val / r_val);
                
            case StandardToken::POW: 
                return Value(std::pow(l_val, r_val));
                
            case StandardToken::MOD: 
                if (r_val == 0.0) {
                    throw RuntimeError(
                        "Modulo by zero at [Row: " + std::to_string(node._token.row) + 
                        ", Col: " + std::to_string(node._token.column) + "]."
                    );
                }
                return Value(std::fmod(l_val, r_val));

            default:
                throw RuntimeError("Unknown binary operator '" + node._token.lexeme + "'.");
        }
    }

    Value MathEvaluator::visit(FunctionNode& node) {
        if (!_context) throw RuntimeError("Evaluation ExecutionContext is null!");

        ICallable* func = _context->getFunction(node._token.lexeme.c_str());

        Vector<Value> evaluated_args;
        evaluated_args.reserve(node._args.size());

        for (const auto& arg_node : node._args) {
            evaluated_args.push_back(arg_node->accept(*this));
        }

        return (*func)(evaluated_args);
    }

}   //  namespace   Synapse
