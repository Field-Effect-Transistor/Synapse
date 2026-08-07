//  /core/src/Parser.cpp
#include "Parser.hpp"

#include "ASTNode.hpp"
#include "Exceptions.hpp"

namespace Synapse::Internal {

    const Token& Parser::_consume(TokenType type, const std::string& error_message) {
        if (_peek().type == type) {
            return _advance();
        }
        Token bad_token = _peek();
        throw SyntaxError(error_message, bad_token.row, bad_token.column);
    }

    ASTNodePtr  Parser::_parseExpression() {
        ASTNodePtr left = _parseTerm();

        while(_match({StandardToken::ADD, StandardToken::SUB})) {
            Token op = _previous();
            ASTNodePtr right = _parseTerm();
            left = ASTNodePtr(new BinaryNode(std::move(op), std::move(left), std::move(right)));
        }

        return left;
    }

    ASTNodePtr  Parser::_parseTerm() {
        ASTNodePtr left = _parsePower();

        while (_match({StandardToken::DIV, StandardToken::MOD, StandardToken::MUL})) {
            Token op = _previous();
            ASTNodePtr right = _parsePower();
            left = ASTNodePtr(new BinaryNode(std::move(op), std::move(left), std::move(right)));
        }

        return left;
    }

    ASTNodePtr  Parser::_parsePower() {
        ASTNodePtr left = _parseFactor();
        if (_check(StandardToken::POW)) {
            Token op = _advance();
            ASTNodePtr right = _parsePower();
            left = ASTNodePtr(new BinaryNode(std::move(op), std::move(left), std::move(right)));
        }
        return left;
    }

    ASTNodePtr  Parser::_parseFactor() {
        if (_match({StandardToken::ADD, StandardToken::SUB})) {
            Token prev = _previous();
            ASTNodePtr  prim = _parseFactor();
            return  ASTNodePtr(new UnaryNode(prev, std::move(prim)));
        }

        ASTNodePtr  prim = _parsePrim();
        while (_match({StandardToken::PERCENT})) {
            Token post = _previous();
            prim = ASTNodePtr(new UnaryNode(std::move(post), std::move(prim)));
        }

        return prim;
    }

    ASTNodePtr Parser::_parsePrim() {
        if (_match({StandardToken::NUMBER})) {
            return ASTNodePtr(new LiteralNode(_previous()));
        }

        if (_match({StandardToken::LPAREN})) {
            ASTNodePtr expression = _parseExpression();
            _consume(StandardToken::RPAREN, "Syntax error: Expected ')' after expression.");
            return expression;
        }

        if (_match({StandardToken::IDENTIFIER})) {
            Token id_token = _previous();

            if (_match({StandardToken::LPAREN})) {
                Vector<ASTNodePtr> args;

                if (!_check(StandardToken::RPAREN)) {
                    do {
                        args.push_back(_parseExpression());
                    } while (_match({StandardToken::COMMA}));
                }

                _consume(StandardToken::RPAREN, "Syntax error: Expected ')' after function arguments.");

                return ASTNodePtr(new FunctionNode(id_token, std::move(args)));
            }

            return ASTNodePtr(new VariableNode(id_token));
        }

        Token bad_token = _peek();
        throw SyntaxError("Unexpected token '" + std::string(bad_token.lexeme) + "'", 
                  bad_token.row, bad_token.column);
    }

    ASTNodePtr Parser::parse(const Vector<Token>& tokens) {
        _tokens = &tokens;
        _current = 0;
        
        if (_isAtEnd()) {
            throw SyntaxError("empty equation!"); 
        }

        ASTNodePtr root = _parseExpression();

        if (!_isAtEnd()) {
            Token bad_token = _peek();
            throw SyntaxError("Unexpected token '" + std::string(bad_token.lexeme) + "' after expression", 
                                bad_token.row, bad_token.column);
        }

        return root;
    }

}   //  namespace   Synapse::Internal
