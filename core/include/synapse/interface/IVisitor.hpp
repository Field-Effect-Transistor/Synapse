//  /core/include/synapse/interface/IVisitor.hpp
#pragma once

#include "synapse/Value.hpp"

namespace Synapse {
    
    struct LiteralNode;
    struct VariableNode;
    struct UnaryNode;
    struct BinaryNode;
    struct FunctionNode;

    struct IVisitor {
        virtual Value visit(LiteralNode& node) = 0;
        virtual Value visit(VariableNode& node) = 0;
        virtual Value visit(BinaryNode& node) = 0;
        virtual Value visit(UnaryNode& node) = 0;
        virtual Value visit(FunctionNode& node) = 0;
    };  //  struct  IVisitor;
}   //  namespace   Synapse
