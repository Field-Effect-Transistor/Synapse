//  /core/include/interface/ASTNode.hpp
#pragma once

#include "interface/IVisitor.hpp"
#include "interface/IASTNode.hpp"
#include "Token.hpp"

#include "utils/include/UniquePtr.hpp"
#include "Vector.hpp"

namespace Hermes {

    struct LiteralNode : ASTNodeBase<LiteralNode> {
        Token   _token;

        LiteralNode(Token token) : _token(std::move(token)) {};
        ~LiteralNode() = default;

        bool compare(const LiteralNode& other) const override {
            return this->_token == other._token; 
        }
    };  //  struct  LiteralNode

    struct VariableNode : ASTNodeBase<VariableNode> {
        Token   _token;

        VariableNode(Token token) : _token(std::move(token)) {};
        ~VariableNode() = default;

        bool compare(const VariableNode& other) const override {
            return this->_token == other._token; 
        }
    };  //  struct  VariableNode

    struct UnaryNode : ASTNodeBase<UnaryNode> {
        Token       _token;
        ASTNodePtr  _child;
        
        UnaryNode(
            Token t,
            ASTNodePtr  child
        ): _token(std::move(t)), _child(std::move(child)) {};
        ~UnaryNode() = default;

        bool compare(const UnaryNode& other) const override {
            return  this->_token == other._token && 
                    this->_child->is_equal(*other._child); 
        }
    };  //  struct  ASTUnanyNode

    struct BinaryNode : ASTNodeBase<BinaryNode> {
        Token       _token;
        ASTNodePtr  _left;
        ASTNodePtr  _right;

        BinaryNode(
            Token       t,
            ASTNodePtr  left, 
            ASTNodePtr  right 
        ) : _token(std::move(t)), _left(std::move(left)), _right(std::move(right))  {}
        ~BinaryNode() = default;

        bool compare(const BinaryNode& other) const override {
            if (this->_token != other._token) return false;
            
            bool left_eq = (this->_left && other._left) ? 
                        (*this->_left == *other._left) : 
                        (this->_left == other._left);
                        
            bool right_eq = (this->_right && other._right) ? 
                            (*this->_right == *other._right) : 
                            (this->_right == other._right);
                            
            return left_eq && right_eq;
        }
    };  //  struct  BinaryNode

    struct FunctionNode : ASTNodeBase<FunctionNode> {
        Token              _token;
        Vector<ASTNodePtr> _args;

        FunctionNode(
            Token               token,
            Vector<ASTNodePtr>  args
        ) : _token(std::move(token)), _args(std::move(args)) {}
        ~FunctionNode() = default;

        bool compare(const FunctionNode& other) const override {
            if (this->_token != other._token) return false;
            if (this->_args.size() != other._args.size()) return false;
            
            for (size_t i = 0; i < this->_args.size(); ++i) {
                if (!this->_args[i]->is_equal(*other._args[i])) {
                    return false;
                }
            }
            return true;
        }
    };  //  FunctionNode

}   //  namespace   Hermes

