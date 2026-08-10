//  /tests/core_tests/test_parser.cpp
#include <gtest/gtest.h>
#include "internal/Parser.hpp"
#include "synapse/ASTPrinter.hpp"
#include "synapse/Exceptions.hpp"

using namespace Synapse;
using namespace Synapse::Internal;

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

TEST_F(ParserPrimitiveTest, ParserThrowsForEmptyTokens) {
    Vector<Token> v = make_vector({});
    Parser parser;
    EXPECT_THROW(parser.parse(v), SyntaxError);
}

TEST_F(ParserPrimitiveTest, ParseParsesSingleNumberIntoLiteralNode) {
    auto v = make_vector({{StandardToken::NUMBER, 42, "42"}});
    Parser parser;
    auto actual = parser.parse(v);

    ASSERT_NE(actual, nullptr);
    
    auto expected = ASTNodePtr(new LiteralNode(
        Token(StandardToken::NUMBER, 42, "42", 0, 0)
    ));
    EXPECT_TRUE(actual->is_equal(*expected));
}

TEST_F(ParserPrimitiveTest, ParseParsesSingleIdentifierIntoVariableNode) {
    auto v = make_vector({{StandardToken::IDENTIFIER, 0, "x"}});
    Parser parser;
    auto actual = parser.parse(v);

    ASSERT_NE(actual, nullptr);

    auto expected = ASTNodePtr(new VariableNode(
        Token(StandardToken::IDENTIFIER, 0, "x", 0, 0)
    ));
    EXPECT_TRUE(actual->is_equal(*expected));
}

TEST_F(ParserPrimitiveTest, ParseParenthesesWrapExpressionCorrectly) {
    auto v = make_vector({
        {StandardToken::LPAREN, 0, "("},
        {StandardToken::NUMBER, 5, "5"},
        {StandardToken::RPAREN, 0, ")"}
    });
    Parser parser;
    auto actual = parser.parse(v);

    ASSERT_NE(actual, nullptr);
    
    auto expected = ASTNodePtr(new LiteralNode(
        Token(StandardToken::NUMBER, 5, "5", 0, 0)
    ));
    EXPECT_TRUE(actual->is_equal(*expected));
}

//  ====================
//      Operators
//  ====================

TEST_F(ParserOperatorTest, ParseCreatesBinaryNodeForAddition) {
    auto v = make_vector({
        {StandardToken::NUMBER, 1, "1"},
        {StandardToken::ADD, 0, "+"},
        {StandardToken::NUMBER, 2, "2"}
    });
    Parser parser;
    auto actual = parser.parse(v);

    ASSERT_NE(actual, nullptr);

    auto expected = ASTNodePtr(new BinaryNode(
        Token(StandardToken::ADD, 0, "+", 0, 0),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 1, "1", 0, 0))),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 2, "2", 0, 0)))
    ));
    EXPECT_TRUE(actual->is_equal(*expected));
}

TEST_F(ParserOperatorTest, ParseRespectsOperatorPrecedence) {
    // 2 + 3 * 4
    auto v = make_vector({
        {StandardToken::NUMBER, 2, "2"},
        {StandardToken::ADD, 0, "+"},
        {StandardToken::NUMBER, 3, "3"},
        {StandardToken::MUL, 0, "*"},
        {StandardToken::NUMBER, 4, "4"}
    });
    Parser parser;
    auto actual = parser.parse(v);

    ASSERT_NE(actual, nullptr);

    // Будуємо множення (* 3 4)
    auto right_mul = ASTNodePtr(new BinaryNode(
        Token(StandardToken::MUL, 0, "*", 0, 0),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 3, "3", 0, 0))),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 4, "4", 0, 0)))
    ));

    // Будуємо корінь (+ 2 (* 3 4))
    auto expected = ASTNodePtr(new BinaryNode(
        Token(StandardToken::ADD, 0, "+", 0, 0),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 2, "2", 0, 0))),
        std::move(right_mul)
    ));
    EXPECT_TRUE(actual->is_equal(*expected));
}

TEST_F(ParserOperatorTest, ParseParenthesesOverridePrecedence) {
    // (2 + 3) * 4
    auto v = make_vector({
        {StandardToken::LPAREN, 0, "("},
        {StandardToken::NUMBER, 2, "2"},
        {StandardToken::ADD, 0, "+"},
        {StandardToken::NUMBER, 3, "3"},
        {StandardToken::RPAREN, 0, ")"},
        {StandardToken::MUL, 0, "*"},
        {StandardToken::NUMBER, 4, "4"}
    });
    Parser parser;
    auto actual = parser.parse(v);

    ASSERT_NE(actual, nullptr);

    // Будуємо додавання (+ 2 3)
    auto left_add = ASTNodePtr(new BinaryNode(
        Token(StandardToken::ADD, 0, "+", 0, 0),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 2, "2", 0, 0))),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 3, "3", 0, 0)))
    ));

    // Будуємо корінь (* (+ 2 3) 4)
    auto expected = ASTNodePtr(new BinaryNode(
        Token(StandardToken::MUL, 0, "*", 0, 0),
        std::move(left_add),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 4, "4", 0, 0)))
    ));
    EXPECT_TRUE(actual->is_equal(*expected));
}

TEST_F(ParserOperatorTest, ParseRespectsLeftAssociativity) {
    // 10 - 4 - 2  =>  (10 - 4) - 2
    auto v = make_vector({
        {StandardToken::NUMBER, 10, "10"},
        {StandardToken::SUB, 0, "-"},
        {StandardToken::NUMBER, 4, "4"},
        {StandardToken::SUB, 0, "-"},
        {StandardToken::NUMBER, 2, "2"}
    });
    Parser parser;
    auto actual = parser.parse(v);

    ASSERT_NE(actual, nullptr);

    // Будуємо лівий вузол (- 10 4)
    auto left_sub = ASTNodePtr(new BinaryNode(
        Token(StandardToken::SUB, 0, "-", 0, 0),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 10, "10", 0, 0))),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 4, "4", 0, 0)))
    ));

    // Будуємо корінь (- (- 10 4) 2)
    auto expected = ASTNodePtr(new BinaryNode(
        Token(StandardToken::SUB, 0, "-", 0, 0),
        std::move(left_sub),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 2, "2", 0, 0)))
    ));
    EXPECT_TRUE(actual->is_equal(*expected));
}

TEST_F(ParserOperatorTest, ParseRespectsRightAssociativityForPower) {
    // 2 ^ 3 ^ 4  =>  2 ^ (3 ^ 4)
    auto v = make_vector({
        {StandardToken::NUMBER, 2, "2"},
        {StandardToken::POW, 0, "^"},
        {StandardToken::NUMBER, 3, "3"},
        {StandardToken::POW, 0, "^"},
        {StandardToken::NUMBER, 4, "4"}
    });
    Parser parser;
    auto actual = parser.parse(v);

    ASSERT_NE(actual, nullptr);

    // Будуємо праве піддерево (^ 3 4)
    auto right_pow = ASTNodePtr(new BinaryNode(
        Token(StandardToken::POW, 0, "^", 0, 0),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 3, "3", 0, 0))),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 4, "4", 0, 0)))
    ));

    // Будуємо корінь (^ 2 (^ 3 4))
    auto expected = ASTNodePtr(new BinaryNode(
        Token(StandardToken::POW, 0, "^", 0, 0),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 2, "2", 0, 0))),
        std::move(right_pow)
    ));
    EXPECT_TRUE(actual->is_equal(*expected));
}

TEST_F(ParserOperatorTest, ParsePrefixAndPostfixUnaryOperators) {
    // - 5 %
    auto v = make_vector({
        {StandardToken::SUB, 0, "-"},
        {StandardToken::NUMBER, 5, "5"},
        {StandardToken::PERCENT, 0, "%"}
    });
    Parser parser;
    auto actual = parser.parse(v);

    ASSERT_NE(actual, nullptr);

    // Будуємо нижній вузол (% 5)
    auto percent_node = ASTNodePtr(new UnaryNode(
        Token(StandardToken::PERCENT, 0, "%", 0, 0),
        ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 5, "5", 0, 0)))
    ));

    // Будуємо корінь (- (% 5))
    auto expected = ASTNodePtr(new UnaryNode(
        Token(StandardToken::SUB, 0, "-", 0, 0),
        std::move(percent_node)
    ));
    EXPECT_TRUE(actual->is_equal(*expected));
}

//  ====================
//      Functions
//  ====================

TEST_F(ParserFunctionTest, ParseFunctionWithNoArguments) {
    // rand()
    auto v = make_vector({
        {StandardToken::IDENTIFIER, 0, "rand"},
        {StandardToken::LPAREN, 0, "("},
        {StandardToken::RPAREN, 0, ")"}
    });
    Parser parser;
    auto actual = parser.parse(v);

    ASSERT_NE(actual, nullptr);

    // Будуємо еталон: порожній вектор аргументів
    Vector<ASTNodePtr> empty_args;
    auto expected = ASTNodePtr(new FunctionNode(
        Token(StandardToken::IDENTIFIER, 0, "rand", 0, 0),
        std::move(empty_args)
    ));

    EXPECT_TRUE(actual->is_equal(*expected));
}

TEST_F(ParserFunctionTest, ParseFunctionWithMultipleArguments) {
    // max(1, 2)
    auto v = make_vector({
        {StandardToken::IDENTIFIER, 0, "max"},
        {StandardToken::LPAREN, 0, "("},
        {StandardToken::NUMBER, 1, "1"},
        {StandardToken::COMMA, 0, ","},
        {StandardToken::NUMBER, 2, "2"},
        {StandardToken::RPAREN, 0, ")"}
    });
    Parser parser;
    auto actual = parser.parse(v);

    ASSERT_NE(actual, nullptr);

    // Збираємо аргументи
    Vector<ASTNodePtr> args;
    args.push_back(ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 1, "1", 0, 0))));
    args.push_back(ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 2, "2", 0, 0))));

    // Будуємо корінь
    auto expected = ASTNodePtr(new FunctionNode(
        Token(StandardToken::IDENTIFIER, 0, "max", 0, 0),
        std::move(args)
    ));

    EXPECT_TRUE(actual->is_equal(*expected));
}

TEST_F(ParserFunctionTest, ParseNestedFunctionCalls) {
    // sin(max(1, x))
    auto v = make_vector({
        {StandardToken::IDENTIFIER, 0, "sin"},
        {StandardToken::LPAREN, 0, "("},
        {StandardToken::IDENTIFIER, 0, "max"},
        {StandardToken::LPAREN, 0, "("},
        {StandardToken::NUMBER, 1, "1"},
        {StandardToken::COMMA, 0, ","},
        {StandardToken::IDENTIFIER, 0, "x"},
        {StandardToken::RPAREN, 0, ")"},
        {StandardToken::RPAREN, 0, ")"}
    });
    Parser parser;
    auto actual = parser.parse(v);

    ASSERT_NE(actual, nullptr);

    // 1. Будуємо аргументи для max: 1 та x
    Vector<ASTNodePtr> max_args;
    max_args.push_back(ASTNodePtr(new LiteralNode(Token(StandardToken::NUMBER, 1, "1", 0, 0))));
    max_args.push_back(ASTNodePtr(new VariableNode(Token(StandardToken::IDENTIFIER, 0, "x", 0, 0))));

    // 2. Будуємо вузол функції max
    auto max_node = ASTNodePtr(new FunctionNode(
        Token(StandardToken::IDENTIFIER, 0, "max", 0, 0),
        std::move(max_args)
    ));

    // 3. Будуємо аргументи для sin: туди входить функція max
    Vector<ASTNodePtr> sin_args;
    sin_args.push_back(std::move(max_node));

    // 4. Будуємо корінь
    auto expected = ASTNodePtr(new FunctionNode(
        Token(StandardToken::IDENTIFIER, 0, "sin", 0, 0),
        std::move(sin_args)
    ));

    EXPECT_TRUE(actual->is_equal(*expected));
}

//  ====================
//      Exceptions
//  ====================

TEST_F(ParserExceptionTest, ThrowsOnMissingClosingParenthesisInExpression) {
    // ( 5 EOF
    auto v = make_vector({
        {StandardToken::LPAREN, 0, "("},
        {StandardToken::NUMBER, 5, "5"}
    });
    Parser parser;
    EXPECT_THROW(parser.parse(v), SyntaxError);
}

TEST_F(ParserExceptionTest, ThrowsOnMissingClosingParenthesisInFunctionCall) {
    // sin( 5 EOF
    auto v = make_vector({
        {StandardToken::IDENTIFIER, 0, "sin"},
        {StandardToken::LPAREN, 0, "("},
        {StandardToken::NUMBER, 5, "5"}
    });
    Parser parser;
    EXPECT_THROW(parser.parse(v), SyntaxError);
}

TEST_F(ParserExceptionTest, ThrowsOnUnexpectedTokenAtStartOfExpression) {
    // * 5
    auto v = make_vector({
        {StandardToken::MUL, 0, "*"},
        {StandardToken::NUMBER, 5, "5"}
    });
    Parser parser;
    EXPECT_THROW(parser.parse(v), SyntaxError);
}

TEST_F(ParserExceptionTest, ThrowsOnTrailingTokensAfterValidExpression) {
    // 2 + 2 3
    auto v = make_vector({
        {StandardToken::NUMBER, 2, "2"},
        {StandardToken::ADD, 0, "+"},
        {StandardToken::NUMBER, 2, "2"},
        {StandardToken::NUMBER, 3, "3"} // Зайвий токен!
    });
    Parser parser;
    EXPECT_THROW(parser.parse(v), SyntaxError);
}
