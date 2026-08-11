//  /core/include/synapse/interface/ILexer.hpp
#pragma once

#include "synapse/Token.hpp"
#include "synapse/interface/IReader.hpp"

#include <cstddef>

namespace Synapse {

    struct ILexer {
        virtual ~ILexer() = default;

        virtual void init(IReader* reader, size_t chunk_size = 1024) = 0;

        virtual Token getNextToken() = 0;
        virtual const Token& peekToken() = 0;
        virtual bool isEOF() = 0;
    };  //  struct  ILexer
}   //  namespace   Synapse
