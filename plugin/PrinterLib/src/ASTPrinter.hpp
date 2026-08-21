//  /plugin/PrinterLib/src/ASTPrinter.hpp
#pragma once

#include "synapse/interface/IVisitor.hpp"
#include "synapse/ASTNode.hpp"

#include "UniquePtr.hpp"

namespace PrinterLib {

    using namespace Synapse;

    class ASTPrinter : public IVisitor {
        struct Impl;
        Synapse::UniquePtr<Impl> _impl;
        
    public:
        ASTPrinter();
        ~ASTPrinter();
        void destroy() override;

        const Role getRole() const override { return Role::Producer; }
        void setContext(ExecutionContext* ctx) override {};

        Value visit(LiteralNode& node) override;
        Value visit(VariableNode& node) override;
        Value visit(BinaryNode& node) override;
        Value visit(UnaryNode& node) override;
        Value visit(FunctionNode& node) override;

        //inline const std::string& result();

    };  //  class   ASTPrinter

}   //  namespace   PrinterLab
