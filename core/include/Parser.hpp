//  /core/include.Parser.hpp
#pragma once

#include "interface/IParser.hpp"

#include "UniquePtr.hpp"
#include "stdexcept"

namespace Hermes {
    class Parser : public IParser {
        const Vector<Token>* _tokens = nullptr;
        size_t  _current = 0;

    public:
        Parser() = delete;

        Parser(const Parser&) = delete;
        Parser& operator=(const Parser&) = delete;

        Parser(Parser&&) = delete;
        Parser& operator=(Parser&&) = delete;

        Parser(Vector<Token>& tokens) : _tokens(&tokens) {}
        ~Parser() = default;

    private:
        const Token& _peek() const { 
            if (_isAtEnd()) return _tokens->back(); 
            return _tokens->operator[](_current); 
        }

        const Token& _previous() const { return _tokens->operator[](_current == 0 ? 0 : _current - 1); }
        
        const Token& _consume(TokenType type, const std::string& error_message) {
            if (_peek().type == type) {
                return _advance();
            }
            throw std::runtime_error(error_message);
        }

        const Token& _advance() {
            if (!_isAtEnd()) {
                ++_current;
            }
            return _previous(); 
        }

        bool _isAtEnd() const {
            if (!_tokens || _current >= _tokens->size()) return true;
            return _tokens->operator[](_current).type == StandardToken::END_OF_FILE;
        }

        bool _check(TokenType type) const { return _peek().type == type; }
        
        bool _match(std::initializer_list<TokenType> types) {
            auto peeked_type = _peek().type;
            for(const auto& type : types) {
                if (type == peeked_type) {
                    _advance();
                    return true;
                }
            }
            return false;
        }

        ASTNodePtr  _parseExpression();
        ASTNodePtr  _parseTerm();
        ASTNodePtr  _parsePower();
        ASTNodePtr  _parseFactor();
        ASTNodePtr  _parsePrim();

    public:
        ASTNodePtr parse(const Vector<Token>& tokens) override {
            _tokens = &tokens;
            _current = 0;
            
            if (_isAtEnd()) return nullptr;

            ASTNodePtr root = _parseExpression();

            if (!_isAtEnd()) {
                Token bad_token = _peek();
                throw std::runtime_error(
                    "Syntax error: Unexpected token '" + std::string(bad_token.lexeme) + 
                    "' after expression at [Row: " + std::to_string(bad_token.row) + 
                    ", Col: " + std::to_string(bad_token.column) + "]."
                );
            }

            return root;
        }

    };  //  class   Parser
}   //  namespace   Hermes
