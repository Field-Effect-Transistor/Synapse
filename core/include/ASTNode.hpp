//  /core/include/interface/ASTNode.hpp
#pragma once

#include "interface/IVisitor.hpp"
#include "interface/IASTNode.hpp"
#include "Token.hpp"

#include "utils/include/UniquePtr.hpp"

#include "Vector.hpp"
#include <memory>

#define ABI_SAFE_ASTNODE_DESTROYER  void destroy() override { delete this; }
#define ASTNODE_VISITOR_ACCEPT      Value accept(IVisitor& v) override { return v.visit(*this); }

namespace Hermes {

    struct ASTNodeDeleter {
        void operator()(IASTNode* ptr) const {
            if (ptr) {
                ptr->destroy();
            }
        }
    };  //  struct ASTNodeDeleter

    using ASTNodePtr = UniquePtr<IASTNode, ASTNodeDeleter>;

    struct LeafNode : IASTNode {
        Value   _value;

        LeafNode(Value value) : _value(std::move(value)) {};
        ~LeafNode() = default;

        ASTNODE_VISITOR_ACCEPT
        ABI_SAFE_ASTNODE_DESTROYER

    };  //  struct  LeafNode

    struct VariableNode : IASTNode {
        Token   _token;

        VariableNode(Token token) : _token(std::move(token)) {};
        ~VariableNode() = default;

        ASTNODE_VISITOR_ACCEPT
        ABI_SAFE_ASTNODE_DESTROYER
    };  //  struct VariableNode

    struct UnaryNode : IASTNode {
        Token       _token;
        ASTNodePtr  _child;
        
        UnaryNode(
            Token t,
            ASTNodePtr  child
        ): _token(std::move(t)), _child(std::move(child)) {};
        ~UnaryNode() = default;

        ASTNODE_VISITOR_ACCEPT
        ABI_SAFE_ASTNODE_DESTROYER
    };  //  struct  ASTUnanyNode

    struct BinaryNode : IASTNode {
        Token       _token;
        ASTNodePtr  _left;
        ASTNodePtr  _right;

        BinaryNode(
            Token       t,
            ASTNodePtr  left, 
            ASTNodePtr  right 
        ) : _token(std::move(t)), _left(std::move(left)), _right(std::move(right))  {}

        ~BinaryNode() = default;

        ASTNODE_VISITOR_ACCEPT
        ABI_SAFE_ASTNODE_DESTROYER
    };  //  BinaryNode

    struct FunctionNode : IASTNode {
        Token                   _token;
        Vector<ASTNodePtr> _args;

        FunctionNode(
            Token                   token,
            Vector<ASTNodePtr> args
        ) : _token(std::move(token)), _args(std::move(args)) {}

        ~FunctionNode() = default;

        ASTNODE_VISITOR_ACCEPT
        ABI_SAFE_ASTNODE_DESTROYER
    };  //  FunctionNode

}   //  namespace   Hermes

#undef  ASTNODE_VISITOR_ACCEPT
#undef  ABI_SAFE_ASTNODE_DESTROYER
