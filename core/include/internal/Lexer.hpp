//  core/include/Lexer.hpp
#pragma once
#include "synapse/interface/ILexer.hpp"

#include <istream>
#include <string>
#include "Vector.hpp"

namespace Synapse::Internal {

    class Lexer final : public ILexer {
        std::istream&   _is;

        Token _token;
        bool  _is_token = false;

        size_t _chunk_size;
        Vector<char>    _chunk;
        size_t  _bytes_read = 0;
        size_t  _pos = 0;

        size_t  _col = 0;
        size_t  _row = 0;  

    inline bool _prayForTheChunk(size_t tail_len = 0) {
        if (_pos < _bytes_read) return true; 
        if (_is.eof()) return false;

        if (tail_len > 0) {
            std::copy(_chunk.begin() + (_bytes_read - tail_len), 
                    _chunk.begin() + _bytes_read, 
                    _chunk.begin());
        }

        if (_chunk.size() < tail_len + _chunk_size) {
            _chunk.resize(_chunk.size() + _chunk_size);
        }

        _is.read(_chunk.data() + tail_len, static_cast<std::streamsize>(_chunk_size));
        
        size_t new_bytes = static_cast<size_t>(_is.gcount());
        _bytes_read = new_bytes + tail_len;
        _pos = 0;

        return _bytes_read > 0 && (new_bytes > 0 || tail_len > 0);
    }

    public:
        Lexer() = delete;

        Lexer(Lexer&&) = delete;
        Lexer& operator=(Lexer&&) = delete;

        Lexer(const Lexer&) = delete;
        Lexer& operator=(const Lexer&) = delete;

        Lexer(std::istream& is, size_t chunk_size = 1024) : _is(is), _chunk_size(chunk_size), _chunk(_chunk_size) {}
        ~Lexer() = default;

        Token fetchNextToken();

        Token getNextToken() override;
        const Token& peekToken() override;
        bool isEOF() override;

    };  //  class   Lexer

}   //  namespace   Synapse::Internal
