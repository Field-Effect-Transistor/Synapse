// /tests/test_lexer.cpp
#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include "internal/Lexer.hpp"
#include "synapse/interface/IReader.hpp"

using namespace Synapse;    
using namespace Synapse::Internal;

class StreamReader : public IReader {
    std::istream& _is;
public:
    StreamReader(std::istream& is) : _is(is) {}

    size_t read(char* buffer, size_t size) override {
        _is.read(buffer, static_cast<std::streamsize>(size));
        return static_cast<size_t>(_is.gcount());
    }

    bool isEOF() const override {
        return _is.eof();
    }
};

TEST(LexerTest, BasicServiceTokens) {
    // Рядок 0: коментар
    // Рядок 1: ; ( ) ,
    std::string code = "// One line comment\n"
                       "; ( ) ,";
    
    // Перевіряємо різні розміри чанків (від дуже малих до повного розміру)
    for (size_t chunk_size = 1; chunk_size <= code.size(); ++chunk_size) {
        std::istringstream stream(code);
        StreamReader reader(stream);
        Lexer lexer;
        lexer.init(&reader, chunk_size);

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
    std::string code = "12.5 + 45 * (2 - .5) / 1.";
    std::istringstream stream(code);
    StreamReader reader(stream);
    Lexer lexer;
    lexer.init(&reader, 15);

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

    t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::LPAREN);

    t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::NUMBER);

    t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::SUB);

    t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::NUMBER);

    t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::RPAREN);

    t  = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::DIV);

    t  = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::NUMBER);

}

TEST(LexerTest, EndOfFileBehavior) {
    std::istringstream stream("");
    StreamReader reader(stream);
    Lexer lexer;
    lexer.init(&reader);

    Token t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::END_OF_FILE);
}

TEST(LexerTest, IdentifiersAndKeywords) {
    std::string code = "My_var1 mod MoD _v";

    for (size_t chunk_size = 1; chunk_size <= code.size(); ++chunk_size) {
        std::istringstream stream(code);
        StreamReader reader(stream);
        Lexer lexer;
        lexer.init(&reader, chunk_size);

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

        // 4. Ключове слово MoD (змішаний регістр)
        t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::IDENTIFIER);
        EXPECT_EQ(t.lexeme, "_v");
    }

    code = "l123456789";
    for (size_t chunk_size = 1; chunk_size <= code.size(); ++chunk_size) {
        std::istringstream stream(code);
        StreamReader reader(stream);
        Lexer lexer;
        lexer.init(&reader, chunk_size);

        Token t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::IDENTIFIER);
        EXPECT_EQ(t.lexeme, "l123456789");
    }
}

TEST(LexerTest, MalformedNumbersAndErrors) {
    std::string code = "12.34.56 100a @";
    
    std::istringstream stream(code);
    StreamReader reader(stream);
    Lexer lexer;
    lexer.init(&reader, 1024);

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
    StreamReader reader(stream);
    Lexer lexer;
    lexer.init(&reader, 1024);

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
        StreamReader reader(stream);
        Lexer lexer;
        lexer.init(&reader, chunk_size);

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
    StreamReader reader(stream);
    Lexer lexer;
    lexer.init(&reader, 1024);

    Token t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::NUMBER);
    EXPECT_DOUBLE_EQ(t.value, 100.0);
    // 100 знаходиться на рядку 0. Позиція: пробіл(1) + табуляція(1) + пробіл(1) = індекс 3
    EXPECT_EQ(t.row, 0);
    EXPECT_EQ(t.column, 3);

    t = lexer.fetchNextToken();
    EXPECT_EQ(t.type, StandardToken::ADD);
    // Плюс знаходиться на рядку 1 (після \n). Позиція: пробіл(1) + табуляція(1) + пробіл(1) = індекс 2
    EXPECT_EQ(t.row, 1);
    EXPECT_EQ(t.column, 3);
}

TEST(LexerTest, EOFDuringTokenParsing) {
    // 1. Односимвольне число на межі EOF (Перевіряє рядок відразу після if (isdigit))
    {
        std::istringstream stream("7");
        StreamReader reader(stream);
        Lexer lexer;
        lexer.init(&reader, 1024);
        Token t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::NUMBER);
        EXPECT_EQ(t.lexeme, "7");
    }

    // 2. Багатосимвольне число на межі EOF (Перевіряє EOF всередині циклу while)
    {
        std::istringstream stream("42.5");
        StreamReader reader(stream);
        Lexer lexer;
        lexer.init(&reader, 1024);
        Token t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::NUMBER);
        EXPECT_EQ(t.lexeme, "42.5");
        EXPECT_DOUBLE_EQ(t.value, 42.5);
    }

    // 3. Односимвольний ідентифікатор на межі EOF
    {
        std::istringstream stream("x");
        StreamReader reader(stream);
        Lexer lexer;
        lexer.init(&reader, 1024);
        Token t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::IDENTIFIER);
        EXPECT_EQ(t.lexeme, "x");
    }

    // 4. Багатосимвольний ідентифікатор на межі EOF
    {
        std::istringstream stream("my_var");
        StreamReader reader(stream);
        Lexer lexer;
        lexer.init(&reader, 1024);
        Token t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::IDENTIFIER);
        EXPECT_EQ(t.lexeme, "my_var");
    }

    // 5. Ключове слово 'mod' точно на межі EOF
    {
        std::istringstream stream("mod");
        StreamReader reader(stream);
        Lexer lexer;
        lexer.init(&reader, 1024);
        Token t = lexer.fetchNextToken();
        EXPECT_EQ(t.type, StandardToken::MOD);
        EXPECT_EQ(t.lexeme, "mod");
    }

    // 6. Звичайне ділення (перевірка гілки else для case '/')
    {
        // Тут після / йде EOF, але перед ним пробіл. 
        std::istringstream stream("10 / ");
        StreamReader reader(stream);
        Lexer lexer;
        lexer.init(&reader, 1024);
        
        Token t1 = lexer.fetchNextToken();
        EXPECT_EQ(t1.type, StandardToken::NUMBER);
        
        Token t2 = lexer.fetchNextToken();
        EXPECT_EQ(t2.type, StandardToken::DIV);
        EXPECT_EQ(t2.lexeme, "/");
    }
}
