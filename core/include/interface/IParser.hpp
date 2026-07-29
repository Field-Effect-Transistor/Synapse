//  /core/include/interface.hpp
#pragma once

#include "IASTNode.hpp"
#include "Token.hpp"

#include "Vector.hpp"

namespace Hermes {
    struct IParser {
        virtual ~IParser() = default;
        virtual ASTNodePtr parse(const Vector<Token>&) = 0;
    };
    
}   //  namespace   Hermes
