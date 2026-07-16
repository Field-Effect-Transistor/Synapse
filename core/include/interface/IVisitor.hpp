//  /core/include/interface/IVisitor.hpp
#pragma once

#include "core/include/Value.hpp"

namespace Hermes {
    
    struct IASTNode;
    struct LeafNode;
    struct UnaryNode;
    struct BinaryNode;
    struct FunctionNode;

    struct IVisitor {
        virtual Value visit (const LeafNode& node) = 0;
        virtual Value visit (const BinaryNode& node) = 0;
        virtual Value visit (const UnaryNode& node) = 0;
        virtual Value visit (const FunctionNode& node) = 0;
    };  //  struct  IVisitor;
}   //  namespace   Hermes


