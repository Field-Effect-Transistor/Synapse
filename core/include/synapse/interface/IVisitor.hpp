//  /core/include/synapse/interface/IVisitor.hpp
#pragma once
#include "IABIObject.hpp"

#include "synapse/Value.hpp"
#include <cstdint>

namespace Synapse {
    class ExecutionContext;
    
    struct LiteralNode;
    struct VariableNode;
    struct UnaryNode;
    struct BinaryNode;
    struct FunctionNode;

    struct IVisitor : IABIObject<IVisitor> {
        enum class Role : uint8_t {
            Preprocessor,
            Producer
        };  

        virtual void setContext(ExecutionContext*) = 0;
        virtual const Role getRole() const = 0;
        virtual Value visit(LiteralNode& node) = 0;
        virtual Value visit(VariableNode& node) = 0;
        virtual Value visit(BinaryNode& node) = 0;
        virtual Value visit(UnaryNode& node) = 0;
        virtual Value visit(FunctionNode& node) = 0;
    };  //  struct  IVisitor;
    
}   //  namespace   Synapse
