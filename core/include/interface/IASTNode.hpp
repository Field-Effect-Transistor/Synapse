//  /core/include/interface/IAstNode.hpp
#pragma once

#include "core/include/Value.hpp"
#include "UniquePtr.hpp"

namespace Hermes {
    struct IVisitor;

    struct IASTNode {
        virtual ~IASTNode() = default;
        virtual void destroy() = 0;

        virtual const void* get_type_id() const = 0;
        virtual bool is_equal(const IASTNode&) const = 0;
        virtual Value accept(IVisitor&) = 0;

        friend bool operator==(const IASTNode& left, const IASTNode& right) {
            return left.is_equal(right);
        }
    };  //  struct  IAstNode

    struct ASTNodeDeleter {
        void operator()(IASTNode* ptr) const {
            if (ptr) {
                ptr->destroy();
            }
        }
    };  //  struct ASTNodeDeleter

    using ASTNodePtr = UniquePtr<IASTNode, ASTNodeDeleter>;

    template<typename DERIVED>
    struct ASTNodeBase : public IASTNode {
        inline static const char type{0};

        virtual bool compare(const DERIVED&) const = 0;

        const void* get_type_id() const override {
            return static_cast<const void*>(&type);
        }

        bool is_equal(const IASTNode& other) const override {
            if (this->get_type_id() != other.get_type_id()) {
                return false;
            }
            const auto& other_derrived = static_cast<const DERIVED&>(other);
            return static_cast<const DERIVED*>(this)->compare(other_derrived);
        }

        void destroy() override {
            delete static_cast<DERIVED*>(this);
        }

        Value accept(IVisitor& v) override {
            return v.visit(static_cast<DERIVED&>(*this));
        }
    };  //  struct  ASTNodeBase

};  //  namespace   Hermes
