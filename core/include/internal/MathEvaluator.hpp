//  /core/include/MathEvaluator.hpp
#pragma once

#include "synapse/interface/IVisitor.hpp"
#include "synapse/ASTNode.hpp"

namespace Synapse::Internal {

    class MathEvaluator final : public IVisitor {
    public:
        Value visit(VariableNode& node) override;
        Value visit(LiteralNode& node) override;
        Value visit(UnaryNode& node) override;
        Value visit(BinaryNode& node) override;
        Value visit(FunctionNode& node) override;

    };  //  class   MathEvaluator

}   //  namespace   Synapse
