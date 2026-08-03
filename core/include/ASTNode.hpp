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

        bool compare(const LiteralNode& other) {
        }
    };  //  struct  LiteralNode

    struct VariableNode : ASTNodeBase<VariableNode> {
        Token   _token;

        VariableNode(Token token) : _token(std::move(token)) {};
        ~VariableNode() = default;
    };  //  struct  VariableNode

    struct UnaryNode : ASTNodeBase<UnaryNode> {
        Token       _token;
        ASTNodePtr  _child;
        
        UnaryNode(
            Token t,
            ASTNodePtr  child
        ): _token(std::move(t)), _child(std::move(child)) {};
        ~UnaryNode() = default;
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
    };  //  struct  BinaryNode

    struct FunctionNode : ASTNodeBase<FunctionNode> {
        Token              _token;
        Vector<ASTNodePtr> _args;

        FunctionNode(
            Token               token,
            Vector<ASTNodePtr>  args
        ) : _token(std::move(token)), _args(std::move(args)) {}

        ~FunctionNode() = default;
    };  //  FunctionNode

}   //  namespace   Hermes

