//  /core/include/synapse/ASTNode.hpp
#pragma once

#include "interface/IVisitor.hpp"
#include "interface/IASTNode.hpp"
#include "Token.hpp"

#include "UniquePtr.hpp"
#include "Vector.hpp"

namespace Synapse {

    template<typename DERIVED>
    struct ASTNodeBase : public IASTNode {
        inline static const char type{0};
        static const void* type_id() { return static_cast<const void*>(&type); }

        virtual bool compare(const DERIVED&) const = 0;

        const void* get_type_id() const override {
            return static_cast<const void*>(&type);
        }

        bool is_equal(const IASTNode& other) const override {
            if (this->get_type_id() != other.get_type_id()) {
                return false;
            }
            const auto& other_derrived = static_cast<const DERIVED&>(other);
            return static_cast<const DERIVED*>(this)->compare(other_derrived);
        }

        void destroy() override {
            delete static_cast<DERIVED*>(this);
        }

        Value accept(IVisitor& v) override {
            return v.visit(static_cast<DERIVED&>(*this));
        }
    };  //  struct  ASTNodeBase

    struct LiteralNode final : ASTNodeBase<LiteralNode> {
        Token   _token;

        LiteralNode(Token token) : _token(std::move(token)) {};
        ~LiteralNode() = default;

        bool compare(const LiteralNode& other) const override {
            return this->_token == other._token; 
        }
    };  //  struct  LiteralNode

    struct VariableNode final : ASTNodeBase<VariableNode> {
        Token   _token;

        VariableNode(Token token) : _token(std::move(token)) {};
        ~VariableNode() = default;

        bool compare(const VariableNode& other) const override {
            return this->_token == other._token; 
        }
    };  //  struct  VariableNode

    struct UnaryNode final : ASTNodeBase<UnaryNode> {
        Token           _token;
        IASTNode::Ptr   _child;
        
        UnaryNode(
            Token t,
            IASTNode::Ptr child
        ): _token(std::move(t)), _child(std::move(child)) {};
        ~UnaryNode() = default;

        bool compare(const UnaryNode& other) const override {
            if (this->_token != other._token) return false;
            
            if (!this->_child && !other._child) return true;
            if (!this->_child || !other._child) return false;
            
            return this->_child->is_equal(*other._child); 
        }
    };  //  struct  ASTUnanyNode

    struct BinaryNode final : ASTNodeBase<BinaryNode> {
        Token           _token;
        IASTNode::Ptr   _left;
        IASTNode::Ptr   _right;

        BinaryNode(
            Token           t,
            IASTNode::Ptr   left, 
            IASTNode::Ptr   right 
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

    struct FunctionNode final : ASTNodeBase<FunctionNode> {
        Token                   _token;
        Vector<IASTNode::Ptr>   _args;

        FunctionNode(
            Token                   token,
            Vector<IASTNode::Ptr>   args
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
    };  //  struct  FunctionNode

}   //  namespace   Synapse
