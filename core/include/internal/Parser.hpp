//  /core/include.Parser.hpp
#pragma once

#include "synapse/interface/IParser.hpp"

#include "UniquePtr.hpp"

namespace Synapse::Internal {
    class Parser final : public IParser {
        const Vector<Token>* _tokens = nullptr;
        size_t  _current = 0;

    public:
        Parser() = default;

        Parser(const Parser&) = delete;
        Parser& operator=(const Parser&) = delete;

        Parser(Parser&&) = delete;
        Parser& operator=(Parser&&) = delete;

        ~Parser() = default;

    private:
        const Token& _peek() const { 
            if (_isAtEnd()) return _tokens->back(); 
            return _tokens->operator[](_current); 
        }

        const Token& _previous() const { return _tokens->operator[](_current == 0 ? 0 : _current - 1); }
        
        const Token& _consume(TokenType type, const std::string& error_message);

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

        IASTNode::Ptr   _parseAssignment();
        IASTNode::Ptr   _parseExpression();
        IASTNode::Ptr   _parseTerm();
        IASTNode::Ptr   _parsePower();
        IASTNode::Ptr   _parseFactor(); 
        IASTNode::Ptr   _parseUnary();
        IASTNode::Ptr   _parsePrim();

        void destroy() override;

    public:
        IASTNode::Ptr parse(const Vector<Token>& tokens) override;

    };  //  class   Parser
}   //  namespace   Synapse::Internal
