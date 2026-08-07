//  /core/include/synapse/interface/IParser.hpp
#pragma once

#include "IASTNode.hpp"
#include "Token.hpp"

#include "Vector.hpp"

namespace Synapse {
    struct IParser {
        virtual ~IParser() = default;
        virtual ASTNodePtr parse(const Vector<Token>&) = 0;
    };
    
}   //  namespace   Synapse
