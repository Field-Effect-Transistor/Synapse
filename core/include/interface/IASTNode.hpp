//  /core/include/interface/IAstNode.hpp
#pragma once

#include "core/include/Value.hpp"
#include "UniquePtr.hpp"

namespace Hermes {
    struct IVisitor;

    struct IASTNode {
        virtual ~IASTNode() = default;
        virtual void destroy() = 0;

        virtual Value accept(IVisitor&) = 0;
    };  //  struct  IAstNode

    struct ASTNodeDeleter {
        void operator()(IASTNode* ptr) const {
            if (ptr) {
                ptr->destroy();
            }
        }
    };  //  struct ASTNodeDeleter

    using ASTNodePtr = UniquePtr<IASTNode, ASTNodeDeleter>;

};  //  namespace   Hermes
