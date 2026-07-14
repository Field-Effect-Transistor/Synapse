// tests/test_lexer.cpp
#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include "Lexer.hpp"

using namespace Hermes;

TEST(LexerTest, BasicServiceTokens) {
    // Рядок 0: коментар
    // Рядок 1: ; ( ) ,
    std::string code = "// One line comment\n"
                       "; ( ) ,";
    
    // Перевіряємо різні розміри чанків (від дуже малих до повного розміру)
    for (size_t chunk_size = 1; chunk_size <= code.size(); ++chunk_size) {
        std::istringstream stream(code);
        Lexer lexer(stream, chunk_size);

        // 1. Коментар
        Token t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::COMMENT);
        EXPECT_EQ(t.lexeme, "// One line comment");
        EXPECT_EQ(t.row, 0);
        EXPECT_EQ(t.column, 0);

        // 2. Крапка з комою
        t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::SEMICOLON);
        EXPECT_EQ(t.lexeme, ";");
        EXPECT_EQ(t.row, 1);
        EXPECT_EQ(t.column, 0);

        // 3. Ліва дужка
        t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::LPAREN);
        EXPECT_EQ(t.lexeme, "(");
        EXPECT_EQ(t.row, 1);
        EXPECT_EQ(t.column, 2); // Перед дужкою є пробіл

        // 4. Права дужка
        t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::RPAREN);
        EXPECT_EQ(t.lexeme, ")");
        EXPECT_EQ(t.row, 1);
        EXPECT_EQ(t.column, 4);

        // 5. Кома
        t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::COMMA);
        EXPECT_EQ(t.lexeme, ",");
        EXPECT_EQ(t.row, 1);
        EXPECT_EQ(t.column, 6);

        // 6. Кінець файлу (EOF)
        t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::END_OF_FILE);
    }
}

TEST(LexerTest, BasicMathTokens) {
    std::string code = "12.5 + 45 * (2 - 0.5)";
    std::istringstream stream(code);
    Lexer lexer(stream, 15);

    Token t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::NUMBER);
    EXPECT_DOUBLE_EQ(t.value, 12.5);
    EXPECT_EQ(t.lexeme, "12.5");
    EXPECT_EQ(t.column, 0);

    t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::ADD);
    EXPECT_EQ(t.lexeme, "+");

    t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::NUMBER);
    EXPECT_DOUBLE_EQ(t.value, 45.0);

    t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::MUL);

}

TEST(LexerTest, EndOfFileBehavior) {
    std::istringstream stream("");
    Lexer lexer(stream);

    Token t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::END_OF_FILE);
}

TEST(LexerTest, IdentifiersAndKeywords) {
    std::string code = "My_var1 mod MoD";

    for (size_t chunk_size = 1; chunk_size <= code.size(); ++chunk_size) {
        std::istringstream stream(code);
        Lexer lexer(stream, chunk_size);

        // 1. Звичайна змінна
        Token t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::IDENTIFIER);
        EXPECT_EQ(t.lexeme, "My_var1");

        // 2. Ключове слово mod (нижній регістр)
        t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::MOD);
        EXPECT_EQ(t.lexeme, "mod");

        // 3. Ключове слово MoD (змішаний регістр)
        t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::MOD);
        EXPECT_EQ(t.lexeme, "MoD");
    }
}

TEST(LexerTest, MalformedNumbersAndErrors) {
    std::string code = "12.34.56 100a @";
    
    std::istringstream stream(code);
    Lexer lexer(stream, 1024);

    // 1. Забагато крапок
    Token t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::UNKNOWN);
    EXPECT_EQ(t.lexeme, "12.34.56");

    // 2. Літера всередині числа
    t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::UNKNOWN);
    EXPECT_EQ(t.lexeme, "100a");

    // 3. Невідомий символ, якого немає в EBNF
    t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::ERROR);
    EXPECT_EQ(t.lexeme, "@");
}

TEST(LexerTest, PublicInterfaceMethods) {
    std::string code = "42 +";
    std::istringstream stream(code);
    Lexer lexer(stream, 1024);

    EXPECT_FALSE(lexer.isEOF());

    // peekToken() можна викликати скільки завгодно разів, він не повинен зміщувати вказівник
    Token p1 = lexer.peekToken();
    Token p2 = lexer.peekToken();
    EXPECT_EQ(p1.type, StandardToken::NUMBER);
    EXPECT_EQ(p2.type, StandardToken::NUMBER);

    // getNextToken() має повернути той самий токен 42 і скинути кеш
    Token g1 = lexer.getNextToken();
    EXPECT_EQ(g1.type, StandardToken::NUMBER);
    EXPECT_DOUBLE_EQ(g1.value, 42.0);

    // Наступний токен - це плюс
    Token g2 = lexer.getNextToken();
    EXPECT_EQ(g2.type, StandardToken::ADD);
    EXPECT_EQ(g2.lexeme, "+");

    // Тепер має бути кінець файлу
    EXPECT_TRUE(lexer.isEOF());
}

TEST(LexerTest, DivisionAndRemainingOperators) {
    std::string code = "- % ^ /";
    
    // Розбиваємо на чанки, щоб перевірити, як лексер реагує на символ '/' 
    // перед кінцем буфера, коли він не знає, чи буде далі ще один '/'
    for (size_t chunk_size = 1; chunk_size <= code.size(); ++chunk_size) {
        std::istringstream stream(code);
        Lexer lexer(stream, chunk_size);

        Token t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::SUB);

        t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::PERCENT);

        t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::POW);

        t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::DIV); // Має розпізнатись як ділення, а не коментар

        t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::END_OF_FILE);
    }
}

TEST(LexerTest, IgnoresWhitespacesAndCarriageReturns) {
    // Рядок містить пробіли, табуляції (\t) та Windows-style переноси (\r\n)
    std::string code = " \t 100 \r\n \t +";
    std::istringstream stream(code);
    Lexer lexer(stream, 1024);

    Token t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::NUMBER);
    EXPECT_DOUBLE_EQ(t.value, 100.0);
    // 100 знаходиться на рядку 0. Позиція: пробіл(1) + табуляція(1) = індекс 2
    EXPECT_EQ(t.row, 0);
    EXPECT_EQ(t.column, 2);

    t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::ADD);
    // Плюс знаходиться на рядку 1 (після \n). Позиція: пробіл(1) + табуляція(1) = індекс 2
    EXPECT_EQ(t.row, 1);
    EXPECT_EQ(t.column, 2);
}
