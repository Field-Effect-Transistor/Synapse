//  /core/include/ILexer.hpp
#pragma once

#include "Token.hpp"

#include <cstddef>
#include <vector>

namespace Synapse {
    //size_t* (*func)(char* chunk, size_t chunk_size) = nullptr;

    struct ILexer {
        virtual ~ILexer() = default;

//        virtual void loadRule(LexerBrainFunc) = 0;    //  temporarly denied
        virtual Token getNextToken() = 0;
        virtual const Token& peekToken() = 0;
        virtual bool isEOF() = 0;
    };  //  struct  ILexer
}   //  namespace   Synapse
