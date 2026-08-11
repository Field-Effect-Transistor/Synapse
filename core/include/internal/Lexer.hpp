//  /core/include/internal/Lexer.hpp
#pragma once

#include "synapse/interface/ILexer.hpp"
#include "synapse/interface/IReader.hpp"
#include "Vector.hpp"

namespace Synapse::Internal {

    class Lexer final : public ILexer {
        IReader* _reader = nullptr;

        Token _token;
        bool  _is_token = false;

        size_t _chunk_size = 0;
        Vector<char>    _chunk;
        size_t  _bytes_read = 0;
        size_t  _pos = 0;

        size_t  _col = 0;
        size_t  _row = 0;  

        inline bool _prayForTheChunk(size_t tail_len = 0) {
            if (_pos < _bytes_read) return true; 
            
            if (!_reader || _reader->isEOF()) return false;

            if (tail_len > 0) {
                std::copy(_chunk.begin() + (_bytes_read - tail_len), 
                          _chunk.begin() + _bytes_read, 
                          _chunk.begin());
            }

            if (_chunk.size() < tail_len + _chunk_size) {
                _chunk.resize(_chunk.size() + _chunk_size);
            }

            size_t new_bytes = _reader->read(_chunk.data() + tail_len, _chunk_size);
            
            _bytes_read = new_bytes + tail_len;
            _pos = 0;

            return _bytes_read > 0 && (new_bytes > 0 || tail_len > 0);
        }

    public:
        Lexer() = default;
        ~Lexer() = default;

        Lexer(Lexer&&) = delete;
        Lexer& operator=(Lexer&&) = delete;

        Lexer(const Lexer&) = delete;
        Lexer& operator=(const Lexer&) = delete;

        void init(IReader* reader, size_t chunk_size = 1024) override {
            _reader = reader;
            _chunk_size = chunk_size;
            _chunk.resize(_chunk_size);
            
            _bytes_read = 0;
            _pos = 0;
            _col = 0;
            _row = 0;
            _is_token = false;
        }

        Token fetchNextToken();

        Token getNextToken() override;
        const Token& peekToken() override;
        bool isEOF() override;

    };  //  class   Lexer

}   //  namespace   Synapse::Internal
