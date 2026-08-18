// /tests/core_tests/test_ast.cpp
#include <gtest/gtest.h>
#include "synapse/ASTNode.hpp"
#include "synapse/Token.hpp"
#include "Vector.hpp"

using namespace Synapse;
using ASTNodePtr = IASTNode::Ptr;

class ASTNodeTest : public ::testing::Test {
protected:
    Token numToken(double val, const char* lexeme) {
        return Token(StandardToken::NUMBER, val, lexeme, 0, 0);
    }
    
    Token varToken(const char* name) {
        return Token(StandardToken::IDENTIFIER, 0, name, 0, 0);
    }
    
    Token opToken(TokenType type, const char* op) {
        return Token(type, 0, op, 0, 0);
    }
};

using ASTLeafNodesTest = ASTNodeTest;
using ASTOperatorNodesTest = ASTNodeTest;
using ASTFunctionNodesTest = ASTNodeTest;


TEST_F(ASTLeafNodesTest, LiteralNodesEquality) {
    ASTNodePtr a(new LiteralNode(numToken(5.0, "5")));
    ASTNodePtr b(new LiteralNode(numToken(5.0, "5")));
    ASTNodePtr c(new LiteralNode(numToken(10.0, "10")));

    EXPECT_TRUE(*a == *b);
    EXPECT_FALSE(*a == *c);
    EXPECT_TRUE(*a != *c);
}

TEST_F(ASTLeafNodesTest, VariableNodesEquality) {
    ASTNodePtr x1(new VariableNode(varToken("x")));
    ASTNodePtr x2(new VariableNode(varToken("x")));
    ASTNodePtr y(new VariableNode(varToken("y")));

    EXPECT_TRUE(*x1 == *x2);
    EXPECT_TRUE(*x1 != *y);
    
    ASTNodePtr fake_var(new LiteralNode(varToken("x")));
    EXPECT_FALSE(*x1 == *fake_var);
}


TEST_F(ASTOperatorNodesTest, UnaryNodesEquality) {
    ASTNodePtr u1(new UnaryNode(
        opToken(StandardToken::SUB, "-"), 
        ASTNodePtr(new LiteralNode(numToken(5.0, "5")))
    ));
    
    ASTNodePtr u2(new UnaryNode(
        opToken(StandardToken::SUB, "-"), 
        ASTNodePtr(new LiteralNode(numToken(5.0, "5")))
    ));
    
    ASTNodePtr u3(new UnaryNode(
        opToken(StandardToken::ADD, "+"), 
        ASTNodePtr(new LiteralNode(numToken(5.0, "5")))
    ));

    EXPECT_TRUE(*u1 == *u2);
    EXPECT_TRUE(*u1 != *u3);
}

TEST_F(ASTOperatorNodesTest, BinaryNodesEquality) {
    ASTNodePtr b1(new BinaryNode(
        opToken(StandardToken::ADD, "+"),
        ASTNodePtr(new LiteralNode(numToken(2.0, "2"))),
        ASTNodePtr(new LiteralNode(numToken(3.0, "3")))
    ));

    ASTNodePtr b2(new BinaryNode(
        opToken(StandardToken::ADD, "+"),
        ASTNodePtr(new LiteralNode(numToken(2.0, "2"))),
        ASTNodePtr(new LiteralNode(numToken(3.0, "3")))
    ));

    ASTNodePtr b3(new BinaryNode(
        opToken(StandardToken::ADD, "+"),
        ASTNodePtr(new LiteralNode(numToken(2.0, "2"))),
        ASTNodePtr(new LiteralNode(numToken(4.0, "4")))
    ));

    EXPECT_TRUE(*b1 == *b2);
    EXPECT_TRUE(*b1 != *b3);
}

TEST_F(ASTOperatorNodesTest, ComplexMathematicalTreeEquality) {
    ASTNodePtr left1(new BinaryNode(
        opToken(StandardToken::ADD, "+"),
        ASTNodePtr(new UnaryNode(opToken(StandardToken::SUB, "-"), ASTNodePtr(new LiteralNode(numToken(5.0, "5"))))),
        ASTNodePtr(new LiteralNode(numToken(3.0, "3")))
    ));
    ASTNodePtr root1(new BinaryNode(
        opToken(StandardToken::MUL, "*"),
        std::move(left1),
        ASTNodePtr(new LiteralNode(numToken(2.0, "2")))
    ));

    ASTNodePtr left2(new BinaryNode(
        opToken(StandardToken::ADD, "+"),
        ASTNodePtr(new UnaryNode(opToken(StandardToken::SUB, "-"), ASTNodePtr(new LiteralNode(numToken(5.0, "5"))))),
        ASTNodePtr(new LiteralNode(numToken(3.0, "3")))
    ));
    ASTNodePtr root2(new BinaryNode(
        opToken(StandardToken::MUL, "*"),
        std::move(left2),
        ASTNodePtr(new LiteralNode(numToken(2.0, "2")))
    ));

    EXPECT_TRUE(*root1 == *root2);
}


TEST_F(ASTFunctionNodesTest, FunctionNodesEquality) {
    Vector<ASTNodePtr> args1;
    args1.push_back(ASTNodePtr(new LiteralNode(numToken(2.0, "2"))));
    args1.push_back(ASTNodePtr(new VariableNode(varToken("x"))));
    ASTNodePtr f1(new FunctionNode(varToken("max"), std::move(args1)));

    Vector<ASTNodePtr> args2;
    args2.push_back(ASTNodePtr(new LiteralNode(numToken(2.0, "2"))));
    args2.push_back(ASTNodePtr(new VariableNode(varToken("x"))));
    ASTNodePtr f2(new FunctionNode(varToken("max"), std::move(args2)));

    Vector<ASTNodePtr> args3;
    args3.push_back(ASTNodePtr(new LiteralNode(numToken(2.0, "2"))));
    args3.push_back(ASTNodePtr(new VariableNode(varToken("y"))));
    ASTNodePtr f3(new FunctionNode(varToken("max"), std::move(args3)));

    EXPECT_TRUE(*f1 == *f2);
    EXPECT_TRUE(*f1 != *f3);
}
