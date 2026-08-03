//  /tests/core_tests/test_parser.cpp
#include <gtest/gtest.h>
#include "Parser.hpp"
#include "ASTPrinter.hpp"

using namespace Hermes;

class ParserTest : public testing::Test {
protected:
    struct T {
        TokenType   type;
        double      value;
        std::string lexeme;
    };

    Vector<Token> make_vector(std::initializer_list<T> list) {
        Vector<Token> res;
        res.reserve(list.size() + 1);
        for (const auto& m : list) {
            res.push_back(Token(m.type, m.value, m.lexeme, 0, 0));
        }
        res.push_back(Token(StandardToken::END_OF_FILE, 0, "", 0, 0));
        return res;
    }

    std::string ast_to_string(ASTNodePtr& tree) {
        static ASTPrinter printer;
        if (!tree) {
            return "null";
        }
        printer.clear();
        tree->accept(printer);
        return printer.result();
    }
    
};  //  class   ParserTest

using ParserPrimitiveTest = ParserTest;
using ParserOperatorTest  = ParserTest;
using ParserFunctionTest  = ParserTest;
using ParserExceptionTest = ParserTest;


//  ====================
//      Primitive
//  ====================
TEST_F(ParserPrimitiveTest, ParserReturnsNullForEmpty) {
    Vector<Token> v = make_vector({});
    
    Parser parser;
    auto ast = parser.parse(v);
    EXPECT_EQ(ast, nullptr);

}

TEST_F(ParserPrimitiveTest, ParseParsesSingleNumberIntoLiteralNode) {
    auto v = make_vector({{StandardToken::NUMBER, 42, "42"}});
    Parser parser;
    auto ast = parser.parse(v);

    ASSERT_NE(ast, nullptr) << "AST should not be null";
    
    std::string result = ast_to_string(ast);
    EXPECT_EQ(result, "42");
}

/*

### 2. Група: Примітиви (`ParserPrimitiveTest`)

Тут тестуємо базові цеглинки (числа, змінні, порожній ввід).

```cpp
TEST_F(ParserPrimitiveTest, ParseReturnsNullptrForEmptyTokens) {
    // 1. Створити вектор токенів, що містить лише END_OF_FILE.
    // 2. Переконатися, що parser.parse() повертає nullptr.
}

TEST_F(ParserPrimitiveTest, ParseParsesSingleNumberIntoLiteralNode) {
    // 1. Вектор з одного числа: NUMBER("42"), END_OF_FILE.
    // 2. Очікуємо корінь, який є LiteralNode.
    // 3. Через ASTPrinter перевіряємо, що результат "42" (або як він у тебе форматується).
}

TEST_F(ParserPrimitiveTest, ParseParsesSingleIdentifierIntoVariableNode) {
    // 1. Вектор з ідентифікатора: IDENTIFIER("x"), END_OF_FILE.
    // 2. Очікуємо VariableNode.
}

TEST_F(ParserPrimitiveTest, ParseParenthesesWrapExpressionCorrectly) {
    // 1. Вектор: LPAREN, NUMBER("5"), RPAREN, END_OF_FILE.
    // 2. Дерево має бути таким самим, як і просто для числа "5" (дужки не створюють окремого вузла).
}

```

---

### 3. Група: Оператори та Пріоритет (`ParserOperatorTest`)

Тут тестуємо бінарні та унарні операції, а головне — чи правильно працює пріоритет (precedence) і асоціативність (associativity).

```cpp
TEST_F(ParserOperatorTest, ParseCreatesBinaryNodeForAddition) {
    // 1. Вектор: NUMBER("1"), ADD, NUMBER("2"), EOF.
    // 2. Перевіряємо, що корінь BinaryNode(+), діти 1 і 2. (напр. "( + 1 2 )")
}

TEST_F(ParserOperatorTest, ParseRespectsOperatorPrecedence) {
    // 1. Вектор для виразу "2 + 3 * 4".
    // 2. Множення має вищий пріоритет.
    // 3. Очікуване дерево через принтер: "( + 2 ( * 3 4 ) )".
}

TEST_F(ParserOperatorTest, ParseParenthesesOverridePrecedence) {
    // 1. Вектор для виразу "( 2 + 3 ) * 4".
    // 2. Очікуване дерево: "( * ( + 2 3 ) 4 )".
}

TEST_F(ParserOperatorTest, ParseRespectsLeftAssociativityForTermAndExpression) {
    // 1. Вектор для виразу "10 - 4 - 2".
    // 2. Має розпарситися як "( - ( - 10 4 ) 2 )", а не "( - 10 ( - 4 2 ) )".
}

TEST_F(ParserOperatorTest, ParseRespectsRightAssociativityForPowerOperator) {
    // 1. Вектор для "2 ^ 3 ^ 4".
    // 2. Твій код використовує рекурсію вправо для POW.
    // 3. Має бути "( ^ 2 ( ^ 3 4 ) )".
}

TEST_F(ParserOperatorTest, ParsePrefixAndPostfixUnaryOperators) {
    // 1. Вектор для "- 5 %".
    // 2. Відсоток (POSTFIX) у тебе в `_parseFactor`, а мінус (PREFIX) теж там.
    // 3. Перевір, як саме формується дерево. Має бути щось на зразок "( - ( % 5 ) )".
}

```

---

### 4. Група: Функції (`ParserFunctionTest`)

Перевіряємо виклики функцій з різною кількістю аргументів.

```cpp
TEST_F(ParserFunctionTest, ParseFunctionWithNoArguments) {
    // 1. Вектор: IDENTIFIER("rand"), LPAREN, RPAREN, EOF.
    // 2. Очікуємо FunctionNode без аргументів.
}

TEST_F(ParserFunctionTest, ParseFunctionWithMultipleArguments) {
    // 1. Вектор: IDENTIFIER("max"), LPAREN, NUMBER("1"), COMMA, NUMBER("2"), RPAREN, EOF.
    // 2. Очікуємо FunctionNode з ім'ям "max" та двома дітьми-аргументами.
}

TEST_F(ParserFunctionTest, ParseNestedFunctionCalls) {
    // 1. Вектор для "sin(max(1, x))".
    // 2. Очікуємо функцію в аргументі іншої функції.
}

```

---

### 5. Група: Обробка помилок (`ParserExceptionTest`)

Оскільки парсер кидає `std::runtime_error`, використовуємо `EXPECT_THROW`.

```cpp
TEST_F(ParserExceptionTest, ThrowsOnMissingClosingParenthesisInExpression) {
    // 1. Вектор: LPAREN, NUMBER("5"), EOF (забули RPAREN).
    // 2. EXPECT_THROW(parser.parse(tokens), std::runtime_error).
    // За бажанням можна юзати EXPECT_THAT(..., ThrowsMessage("...")) з gmock, щоб перевірити текст помилки.
}

TEST_F(ParserExceptionTest, ThrowsOnMissingClosingParenthesisInFunctionCall) {
    // 1. Вектор: IDENTIFIER("sin"), LPAREN, NUMBER("5"), EOF.
    // 2. EXPECT_THROW(..., std::runtime_error).
}

TEST_F(ParserExceptionTest, ThrowsOnUnexpectedTokenAtStartOfExpression) {
    // 1. Вектор: MUL, NUMBER("5"), EOF. (Вираз не може починатися з *)
    // 2. EXPECT_THROW(..., std::runtime_error).
}

TEST_F(ParserExceptionTest, ThrowsOnTrailingTokensAfterValidExpression) {
    // 1. Вектор: NUMBER("2"), ADD, NUMBER("2"), NUMBER("3"), EOF.
    // 2. Парсер успішно з'їсть "2 + 2", а потім побачить "3", коли очікує EOF.
    // 3. EXPECT_THROW(..., std::runtime_error).
}

```
*/



