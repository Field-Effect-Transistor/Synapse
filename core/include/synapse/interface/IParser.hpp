//  /core/include/synapse/interface/IParser.hpp
#pragma once

#include "synapse/interface/IASTNode.hpp"
#include "synapse/Token.hpp"

#include "Vector.hpp"

namespace Synapse {
    struct IParser {
        virtual ~IParser() = default;
        virtual ASTNodePtr parse(const Vector<Token>&) = 0;
    };
    
}   //  namespace   Synapse
