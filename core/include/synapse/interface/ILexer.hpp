//  /core/include/synapse/interface/ILexer.hpp
#pragma once

#include "synapse/Token.hpp"

#include <cstddef>

namespace Synapse {
    /**
     * @brief Lexer interface
     * 
     * Defines the basic contract for transforming a symbol stream
     * into a token sequence for further syntax analysis.
     */
    struct ILexer {
        virtual ~ILexer() = default;

        /**
         * @brief Retrieves and consumes the next token from the stream.
         * 
         * This method modifies the internal state of the lexer by advancing 
         * the pointer. The next call will return a new token.
         * 
         * @return Token The extracted token. If the stream is finished, returns an END_OF_FILE token.
         */
        virtual Token getNextToken() = 0;

        /**
         * @brief Peeks at the next token without consuming it.
         * 
         * Allows for lookahead. This method does not modify the internal 
         * state of the lexer — multiple consecutive calls to peekToken() 
         * will return the same token until getNextToken() is called.
         * 
         * @return const Token& A reference to the next token in the queue.
         */
        virtual const Token& peekToken() = 0;

        /**
         * @brief Checks if the end of the input stream has been reached.
         * 
         * @return true If there are no more tokens to read (the current token is END_OF_FILE).
         * @return false If there are still tokens available in the stream.
         */
        virtual bool isEOF() = 0;
        
    };  //  struct  ILexer
    
}   //  namespace   Synapse
