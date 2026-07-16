//  /core/include/interface/ASTNode.hpp
#pragma once

#include "interface/IVisitor.hpp"
#include "interface/IASTNode.hpp"
#include "Token.hpp"

#include <vector>
#include <memory>

namespace Hermes {
    struct LeafNode : IASTNode {
        Value   value;

        LeafNode(const Value& v) : value(v) {};
        ~LeafNode() {};

        Value accept(IVisitor& v) override {
            return v.visit(*this);
        }
    };  //  struct  LeafNode

    struct UnaryNode : IASTNode {
        Token   _token;
        
        UnaryNode(const Token& t): _token(t) {};
        ~UnaryNode() {};

        Value accept(IVisitor& v) override {
            return v.visit(*this);
        }
    };  //  struct  ASTUnanyNode

    struct BinaryNode : IASTNode {
        Token       _token;
        IASTNode*   _left;
        IASTNode*   _right;

        BinaryNode(
            const Token&    t,
            IASTNode*&      left, 
            IASTNode*&      right 
        ) : _token(t) {
            _left = left;
            left = 0;

            _right = right;
            right = 0;
        }

        ~BinaryNode() {
            delete _left;
            delete _right;
        };
    };  //  BinaryNode

    struct FunctionNode : IASTNode {
        std::string             _name;
        std::vector<IASTNode*>  _args;

        FunctionNode(std::string& n) : _name(n) {
            
        }

        ~FunctionNode() {
            for (auto& arg : args) {
                delete arg;
            }
        };
    };  //  FunctionNode

}   //  namespace   Hermes
