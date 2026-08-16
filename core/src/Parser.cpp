//  /core/src/Parser.cpp
#include "internal/Parser.hpp"

#include "synapse/ASTNode.hpp"
#include "synapse/Exceptions.hpp"

namespace Synapse::Internal {

    const Token& Parser::_consume(TokenType type, const std::string& error_message) {
        if (_peek().type == type) {
            return _advance();
        }
        Token bad_token = _peek();
        throw SyntaxError(error_message, bad_token.row, bad_token.column);
    }

    IASTNode::Ptr   Parser::_parseExpression() {
        IASTNode::Ptr  left = _parseTerm();

        while(_match({StandardToken::ADD, StandardToken::SUB})) {
            Token op = _previous();
            IASTNode::Ptr  right = _parseTerm();
            left = IASTNode::Ptr(new BinaryNode(std::move(op), std::move(left), std::move(right)));
        }

        return left;
    }

    IASTNode::Ptr   Parser::_parseTerm() {
        IASTNode::Ptr  left = _parsePower();

        while (_match({StandardToken::DIV, StandardToken::MOD, StandardToken::MUL})) {
            Token op = _previous();
            IASTNode::Ptr  right = _parsePower();
            left = IASTNode::Ptr(new BinaryNode(std::move(op), std::move(left), std::move(right)));
        }

        return left;
    }

    IASTNode::Ptr   Parser::_parsePower() {
        IASTNode::Ptr  left = _parseFactor();
        if (_check(StandardToken::POW)) {
            Token op = _advance();
            IASTNode::Ptr  right = _parsePower();
            left = IASTNode::Ptr(new BinaryNode(std::move(op), std::move(left), std::move(right)));
        }
        return left;
    }

    IASTNode::Ptr   Parser::_parseFactor() {
        if (_match({StandardToken::ADD, StandardToken::SUB})) {
            Token prev = _previous();
            IASTNode::Ptr   prim = _parseFactor();
            return  IASTNode::Ptr(new UnaryNode(prev, std::move(prim)));
        }

        IASTNode::Ptr   prim = _parsePrim();
        while (_match({StandardToken::PERCENT})) {
            Token post = _previous();
            prim = IASTNode::Ptr(new UnaryNode(std::move(post), std::move(prim)));
        }

        return prim;
    }

    IASTNode::Ptr  Parser::_parsePrim() {
        if (_match({StandardToken::NUMBER})) {
            return IASTNode::Ptr(new LiteralNode(_previous()));
        }

        if (_match({StandardToken::LPAREN})) {
            IASTNode::Ptr  expression = _parseExpression();
            _consume(StandardToken::RPAREN, "Syntax error: Expected ')' after expression.");
            return expression;
        }

        if (_match({StandardToken::IDENTIFIER})) {
            Token id_token = _previous();

            if (_match({StandardToken::LPAREN})) {
                Vector<IASTNode::Ptr> args;

                if (!_check(StandardToken::RPAREN)) {
                    do {
                        args.push_back(_parseExpression());
                    } while (_match({StandardToken::COMMA}));
                }

                _consume(StandardToken::RPAREN, "Syntax error: Expected ')' after function arguments.");

                return IASTNode::Ptr(new FunctionNode(id_token, std::move(args)));
            }

            return IASTNode::Ptr(new VariableNode(id_token));
        }

        Token bad_token = _peek();
        throw SyntaxError("Unexpected token '" + std::string(bad_token.lexeme) + "'", 
                  bad_token.row, bad_token.column);
    }

    IASTNode::Ptr  Parser::parse(const Vector<Token>& tokens) {
        _tokens = &tokens;
        _current = 0;
        
        if (_isAtEnd()) {
            throw SyntaxError("empty equation!"); 
        }

        IASTNode::Ptr  root = _parseExpression();

        if (!_isAtEnd()) {
            Token bad_token = _peek();
            throw SyntaxError("Unexpected token '" + std::string(bad_token.lexeme) + "' after expression", 
                                bad_token.row, bad_token.column);
        }

        return root;
    }

    void Parser::destroy() {
        delete this;
    }

}   //  namespace   Synapse::Internal
