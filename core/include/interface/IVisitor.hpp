//  /core/include/interface/IVisitor.hpp
#pragma once

#include "core/include/Value.hpp"

namespace Hermes {
    
    struct LeafNode;
    struct VariableNode;
    struct UnaryNode;
    struct BinaryNode;
    struct FunctionNode;

    struct IVisitor {
        virtual Value visit(LeafNode& node) = 0;
        virtual Value visit(VariableNode& node) = 0;
        virtual Value visit(BinaryNode& node) = 0;
        virtual Value visit(UnaryNode& node) = 0;
        virtual Value visit(FunctionNode& node) = 0;
    };  //  struct  IVisitor;
}   //  namespace   Hermes


