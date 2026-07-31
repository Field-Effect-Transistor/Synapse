//  /core/include/MathEvaluator.hpp
#pragma once

#include "interface/IVisitor.hpp"
#include "ASTNode.hpp"

#include <string>

#include <cmath>
#include <stdexcept>

namespace Hermes {
    
    class MathEvaluator : public IVisitor {
        Value visit(VariableNode& node) override {
            //  відкладаю на далеке майбутнє)
            throw std::runtime_error("[ERROR] MathEvaluator:IVisitor::visit(VariableNode&): unsupported ast node");
        }

        Value visit(LiteralNode& node) override {
            return Value(node._token.value);
        }

        Value visit(UnaryNode& node) override {
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
                    throw std::runtime_error("Unsupported unary operator: " + node._token.lexeme);
            }

            return Value(value);
        }

        Value visit(BinaryNode& node) override {
            Value left = node._left->accept(*this);
            Value right = node._right->accept(*this);

            TokenType type = node._token.type;

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
                        throw std::runtime_error(
                            "Math error: Division by zero at [Row: " + std::to_string(node._token.row) + 
                            ", Col: " + std::to_string(node._token.column) + "]."
                        );
                    }
                    return Value(l_val / r_val);
                    
                case StandardToken::POW: 
                    return Value(std::pow(l_val, r_val));
                    
                case StandardToken::MOD: 
                    if (r_val == 0.0) {
                        throw std::runtime_error("Math error: Modulo by zero.");
                    }
                    return Value(std::fmod(l_val, r_val));

                default:
                    throw std::runtime_error("Runtime error: Unknown binary operator '" + node._token.lexeme + "'.");
            }
        }

        Value visit(FunctionNode& node) override {
            throw std::runtime_error("[ERROR] MathEvaluator:IVisitor::visit(FunctionNode&): unsupported ast node");
        }
    };  //  class   MathEvaluator

}   //  namespace   Hermes
