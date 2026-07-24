//  /core/include/interface/IAstNode.hpp
#pragma once

#include "core/include/Value.hpp"

namespace Hermes {
    struct IVisitor;

    struct IASTNode {
        virtual ~IASTNode() = default;
        virtual void destroy() = 0;

        virtual Value accept(IVisitor&) = 0;
    };  //  struct  IAstNode

};  //  namespace   Hermes
