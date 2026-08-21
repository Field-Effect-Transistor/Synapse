//  /core/include/MathEvaluator.hpp
#pragma once

#include "synapse/interface/IVisitor.hpp"
#include "synapse/ASTNode.hpp"

namespace Synapse {
    class ExecutionContext;
}   //  namespace   Synapse

namespace Synapse::Internal {

    class MathEvaluator final : public IVisitor {
        ExecutionContext* _context = nullptr;
    public:
        MathEvaluator(ExecutionContext* ctx) : _context(ctx) {}
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

        const Role getRole() const override { return Role::Producer; }
        void setContext(ExecutionContext* ctx) override;

        Value visit(VariableNode& node) override;
        Value visit(LiteralNode& node) override;
        Value visit(UnaryNode& node) override;
        Value visit(BinaryNode& node) override;
        Value visit(FunctionNode& node) override;

    };  //  class   MathEvaluator

}   //  namespace   Synapse
