//  /core/include/synapse/interface/IAstNode.hpp
#pragma once
#include "IABIObject.hpp"

#include "synapse/Value.hpp"

namespace Synapse {
    struct IVisitor;

    struct IASTNode : IABIObject<IASTNode> {
        virtual ~IASTNode() = default;

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

};  //  namespace   Synapse
