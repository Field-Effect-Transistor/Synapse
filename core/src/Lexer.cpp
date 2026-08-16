//  /core/src/Lexer.cpp
#include "internal/Lexer.hpp"

#include <cctype>
#include <cstring>
#include <algorithm>

static int stricmp(char const *a, char const *b) {
    for(;;++a, ++b) {
        int d = tolower(static_cast<unsigned int>(*a)) - tolower(static_cast<unsigned int>(*b));
        if (d != 0 || !*a) {
            return d;
        }
    }

    return 0;
}

namespace Synapse::Internal {
    Token Lexer::fetchNextToken() {
        Token token;

        while(true) {
            
        //  pull new chunk
        if (!_prayForTheChunk()) {
            return Token(StandardToken::END_OF_FILE, 0, "EOF", _col, _row);
        }
            switch (_chunk[_pos]) {
                case '\n': {
                    ++_pos;
                    ++_row;
                    _col = 0;
                    continue;
                }
                case '\t':
                case '\r':
                case ' ': {
                    ++_pos;
                    ++_col;
                    continue;
                }
                case '+': {
                    token = Token(StandardToken::ADD, 0, "+", _col, _row);
                    break;
                }
                case '-': {
                    token = Token(StandardToken::SUB, 0, "-", _col, _row);
                    break;
                }
                case '*': {
                    token = Token(StandardToken::MUL, 0, "*", _col, _row);
                    break;
                }
                case '/': {
                    auto start_col = _col, start_row = _row;
                    token.type = StandardToken::COMMENT; 
                    token.lexeme = "/";

                    ++_pos; ++_col;

                    if (!_prayForTheChunk()) {
                        return Token(StandardToken::DIV, 0, "/", start_col, start_row);
                    }

                    if (_chunk[_pos] == '/') {
                        token.lexeme += "/";
                        ++_pos; ++_col;

                        while (true) {
                            if (!_prayForTheChunk()) return token;
                            
                            char c = _chunk[_pos];
                            if (c == '\n') {
                                break;
                            }
                            if (c != '\r') {
                                token.lexeme += c;
                            }
                            
                            ++_pos; ++_col;
                        }
                        
                        ++_row; _col = 0; ++_pos; 
                        return token;
                    } else {
                        return Token(StandardToken::DIV, 0, "/", start_col, start_row);
                    }
                }
                case '%': {
                    token = Token(StandardToken::PERCENT, 0, "%", _col, _row);
                    break;
                }
                case '^': {
                    token = Token(StandardToken::POW, 0, "^", _col, _row);
                    break;
                }
                case '(': {
                    token = Token(StandardToken::LPAREN, 0, "(", _col, _row);
                    break;
                }
                case ')': {
                    token = Token(StandardToken::RPAREN, 0, ")", _col, _row);
                    break;
                }
                case ',': {
                    token = Token(StandardToken::COMMA, 0, ",", _col, _row);
                    break;
                }
                case ';': {
                    token = Token(StandardToken::SEMICOLON, 0, ";", _col, _row);
                    break;
                }
                default: {
                    if (isdigit(_chunk[_pos]) || _chunk[_pos] == '.') {
                        size_t dot_counter = (_chunk[_pos] == '.') ? 1 : 0;
                        bool valid_token = true;

                        token.lexeme = _chunk[_pos];
                        token.row = _row;
                        token.column = _col;
                        ++_pos; ++_col;

                        while (true) {
                            if (!_prayForTheChunk()) {
                                break;
                            }
                            
                            char c = _chunk[_pos];
                            if (c == '.') {
                                token.lexeme += c;
                                ++dot_counter;
                            } else if (isdigit(c)) {
                                token.lexeme += c;
                            } else if (isalpha(c) || c == '_') {
                                valid_token = false;
                                token.lexeme += c;
                            } else {
                                break;
                            }
                            
                            ++_pos; ++_col;
                        }

                        if (valid_token && dot_counter <= 1 && token.lexeme != ".") {
                            token.type = StandardToken::NUMBER;
                            try {
                                token.value = std::stod(token.lexeme);
                            } catch (const std::out_of_range&) {
                                token.type = StandardToken::ERROR;
                            }
                        } else {
                            if (token.lexeme == ".") {
                                token.type = StandardToken::ERROR;
                            } else {
                                token.type = StandardToken::UNKNOWN;
                            }
                        }

                        return token;
                    } else if (isalpha(_chunk[_pos]) || _chunk[_pos] == '_') {
                        token.type = StandardToken::IDENTIFIER;
                        token.lexeme = _chunk[_pos];
                        token.column = _col;
                        token.row = _row;
                        ++_pos; ++_col;

                        if (!_prayForTheChunk()) {
                            return token;
                        }

                        while (true) {
                            if (!_prayForTheChunk()) {
                                if(!stricmp(token.lexeme.c_str(), "mod")) {
                                    token.type = StandardToken::MOD;
                                }
                                return token;
                            }

                            char c = _chunk[_pos];
                            if (isalpha(c) || c == '_' || isdigit(c)) {
                                token.lexeme += c;
                            } else {
                                break;
                            }

                            ++_pos; ++_col;
                        }

                        if(!stricmp(token.lexeme.c_str(), "mod")) {
                            token.type = StandardToken::MOD;
                        }

                        return token;
                        
                    } else {
                        token = Token(StandardToken::ERROR, 0, std::string(1, _chunk[_pos]), _col, _row);
                    }
                    break;
                }
            }

            ++_col;
            ++_pos;
            return token;
        }
    }

    const Token& Lexer::peekToken() {
        if (_is_token) {
            return _token;
        } else {
            _token = fetchNextToken();
            _is_token = true;
            return _token;
        }
    }

    Token Lexer::getNextToken() {
    if (_is_token) {
        _is_token = false;
        return std::move(_token); 
    } else {
        return fetchNextToken();
    }
}

    bool Lexer::isEOF() {
        return peekToken().type == StandardToken::END_OF_FILE;
    }

    void Lexer::destroy() {
        delete this;
    }

}   //  namespace   Synapse::Internal
