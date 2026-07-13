//  include/core/Token.hpp
#pragma once

#include <cstdint>
#include <string>
#include <iostream> // Для std::ostream

namespace Hermes {
    
    // Повертаємо ідеально простий тип
    using TokenType = uint16_t;

    struct StandardToken {
        //  Service Tokens
        static constexpr TokenType  UNKNOWN     = 0;
        static constexpr TokenType  COMMENT     = 1;
        static constexpr TokenType  END_OF_LINE = 2;
        static constexpr TokenType  END_OF_FILE = 3;
        static constexpr TokenType  SEMICOLON   = 4;
        static constexpr TokenType  LPAREN      = 5;
        static constexpr TokenType  RPAREN      = 6;
        static constexpr TokenType  COMMA       = 7;
        static constexpr TokenType  DOT         = 8;

        //  Errors
        static constexpr TokenType  ERROR       = 10;

        //  Lexeme Tokens
        static constexpr TokenType  NUMBER      = 100;
        static constexpr TokenType  IDENTIFIER  = 101;

        //  Basic operators
        static constexpr TokenType  ADD         = 200;
        static constexpr TokenType  SUB         = 201;
        static constexpr TokenType  MUL         = 202;
        static constexpr TokenType  DIV         = 203;
        static constexpr TokenType  MOD         = 204;
        static constexpr TokenType  POW         = 205;

        static constexpr TokenType  USER_DEFINED_START = 1000;

        static inline const char* toString(TokenType t) {
            switch (t) {
                case UNKNOWN:     return "UNKNOWN";
                case COMMENT:     return "COMMENT";
                case END_OF_LINE: return "END_OF_LINE";
                case END_OF_FILE: return "END_OF_FILE";
                case SEMICOLON:   return "SEMICOLON";
                case LPAREN:      return "LPAREN";
                case RPAREN:      return "RPAREN";
                case COMMA:       return "COMMA";
                case DOT:         return "DOT";
                case ERROR:       return "ERROR";
                case NUMBER:      return "NUMBER";
                case IDENTIFIER:  return "IDENTIFIER";
                case ADD:         return "ADD";
                case SUB:         return "SUB";
                case MUL:         return "MUL";
                case DIV:         return "DIV";
                case MOD:         return "MOD";
                case POW:         return "POW";
                default:          return "USER_DEFINED"; // Плагіни!
            }
        }
    };  //  struct  StandardToken

    struct Token {
        TokenType   type = StandardToken::UNKNOWN;
        double      value = 0;
        std::string lexeme;

        uint16_t    column = 0;
        uint16_t    row = 0;

        Token() {}
        Token(
            TokenType   t,
            double      v,
            std::string l,
            uint16_t    c,
            uint16_t    r
        ): type(t), value(v), lexeme(std::move(l)), column(c), row(r) {}

        void print(std::ostream& os) const {
            os << "[R:" << row << " C:" << column << "] "
               << StandardToken::toString(type) 
               << " ('" << lexeme << "')";
            
            if (type == StandardToken::NUMBER) {
                os << " Val: " << value;
            }
        }
    };  //  struct  Token
};  //  namespace   Hermes
