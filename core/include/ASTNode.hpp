//  /core/include/interface/ASTNode.hpp
#pragma once

#include "interface/IVisitor.hpp"
#include "interface/IASTNode.hpp"
#include "Token.hpp"

#include "utils/include/UniquePtr.hpp"

#include "Vector.hpp"
#include <memory>

#define _ABI_SAFE_ASTNODE_DESTROYER  void destroy() override { delete this; }
#define _ASTNODE_VISITOR_ACCEPT      Value accept(IVisitor& v) override { return v.visit(*this); }

namespace Hermes {

    struct LiteralNode : IASTNode {
        Token   _token;

        LiteralNode(Token token) : _token(std::move(token)) {};
        ~LiteralNode() = default;

        _ASTNODE_VISITOR_ACCEPT
        _ABI_SAFE_ASTNODE_DESTROYER

    };  //  struct  LiteralNode

    struct VariableNode : IASTNode {
        Token   _token;

        VariableNode(Token token) : _token(std::move(token)) {};
        ~VariableNode() = default;

        _ASTNODE_VISITOR_ACCEPT
        _ABI_SAFE_ASTNODE_DESTROYER
    };  //  struct VariableNode

    struct UnaryNode : IASTNode {
        Token       _token;
        ASTNodePtr  _child;
        
        UnaryNode(
            Token t,
            ASTNodePtr  child
        ): _token(std::move(t)), _child(std::move(child)) {};
        ~UnaryNode() = default;

        _ASTNODE_VISITOR_ACCEPT
        _ABI_SAFE_ASTNODE_DESTROYER
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

        _ASTNODE_VISITOR_ACCEPT
        _ABI_SAFE_ASTNODE_DESTROYER
    };  //  BinaryNode

    struct FunctionNode : IASTNode {
        Token              _token;
        Vector<ASTNodePtr> _args;

        FunctionNode(
            Token                   token,
            Vector<ASTNodePtr> args
        ) : _token(std::move(token)), _args(std::move(args)) {}

        ~FunctionNode() = default;

        _ASTNODE_VISITOR_ACCEPT
        _ABI_SAFE_ASTNODE_DESTROYER
    };  //  FunctionNode

}   //  namespace   Hermes

#undef  _ASTNODE_VISITOR_ACCEPT
#undef  _ABI_SAFE_ASTNODE_DESTROYER
