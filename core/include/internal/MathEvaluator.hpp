//  /core/include/MathEvaluator.hpp
#pragma once

#include "synapse/interface/IVisitor.hpp"
#include "synapse/ASTNode.hpp"

namespace Synapse {
    class Context;
}   //  namespace   Synapse

namespace Synapse::Internal {

    class MathEvaluator final : public IVisitor {
        Context* _context = nullptr;
    public:
        MathEvaluator(Context* ctx) : _context(ctx) {}
        ~MathEvaluator() = default;
        void destroy() override;

        MathEvaluator(const MathEvaluator&) = delete;
        MathEvaluator& operator=(const MathEvaluator&) = delete;

        MathEvaluator(MathEvaluator&& meval) {
            this->_context = meval._context;
            meval._context = nullptr;
        }
        MathEvaluator& operator=(MathEvaluator&& meval) {
            this->_context = meval._context;
            meval._context = nullptr;
            return *this;
        }

        Value visit(VariableNode& node) override;
        Value visit(LiteralNode& node) override;
        Value visit(UnaryNode& node) override;
        Value visit(BinaryNode& node) override;
        Value visit(FunctionNode& node) override;

    };  //  class   MathEvaluator

}   //  namespace   Synapse
