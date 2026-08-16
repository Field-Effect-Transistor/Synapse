//  /core/include/synapse/interface/IParser.hpp
#pragma once
#include "IABIObject.hpp"

#include "synapse/interface/IASTNode.hpp"
#include "synapse/Token.hpp"

#include "Vector.hpp"

namespace Synapse {
    struct IParser : IABIObject<IParser> {
        virtual ~IParser() = default;

        virtual IASTNode::Ptr parse(const Vector<Token>&) = 0;
    };

}   //  namespace   Synapse
