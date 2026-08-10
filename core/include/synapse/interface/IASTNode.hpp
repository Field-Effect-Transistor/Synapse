//  /core/include/synapse/interface/IAstNode.hpp
#pragma once

#include "synapse/Value.hpp"
#include "UniquePtr.hpp"

namespace Synapse {
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

        friend bool operator!=(const IASTNode& left, const IASTNode& right) {
            return !left.is_equal(right);
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
};  //  namespace   Synapse
