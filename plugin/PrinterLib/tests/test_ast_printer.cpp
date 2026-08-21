//  /plugin/PrinterLib/tests/test_ast_printer.cpp
#include <gtest/gtest.h>
#include "ASTPrinter.hpp"
#include "synapse/ASTNode.hpp"

using namespace Synapse;
using namespace PrinterLib;

using ASTNodePtr = IASTNode::Ptr;

class ASTPrinterTest : public ::testing::Test {};

using ASTPrinterFormatTest    = ASTPrinterTest;
using ASTPrinterLifecycleTest = ASTPrinterTest;


//  --------------------
//      Formatting
//  --------------------

TEST_F(ASTPrinterFormatTest, PrintsFlatLispLikeStringCorrectly) {
    ASTNodePtr  _5(new LiteralNode(Token(StandardToken::NUMBER, 5.0, "5", 0, 0))),
                _3(new LiteralNode(Token(StandardToken::NUMBER, 3.0, "3", 0, 0))),
                _2(new LiteralNode(Token(StandardToken::NUMBER, 2.0, "2", 0, 0))),
                _x(new VariableNode(Token(StandardToken::IDENTIFIER, 0, "x", 0, 0)));

    ASTNodePtr  _m5(new UnaryNode(Token(StandardToken::SUB, 0, "-", 0, 0), std::move(_5)));
    
    ASTNodePtr  _m5p3(new BinaryNode(Token(StandardToken::ADD, 0, "+", 0, 0), std::move(_m5), std::move(_3)));

    Vector<ASTNodePtr> max_args;
    max_args.push_back(std::move(_2));
    max_args.push_back(std::move(_x));
    
    ASTNodePtr  _max2x(new FunctionNode(
        Token(StandardToken::IDENTIFIER, 0, "max", 0, 0), 
        std::move(max_args)
    ));

    ASTNodePtr  _root(new BinaryNode(Token(StandardToken::MUL, 0, "*", 0, 0), std::move(_m5p3), std::move(_max2x)));

    ASTPrinter printer;
    Value result_val = _root->accept(printer);

    std::string expected_output = "( * ( + ( - 5 ) 3 ) ( max 2 x ) )";

    ASSERT_TRUE(result_val.isCustom());
    std::string actual_output = result_val.getCustom()->to_str();

    EXPECT_EQ(actual_output, expected_output);
}

TEST_F(ASTPrinterFormatTest, SingleLiteral) {
    ASTNodePtr _42(new LiteralNode(Token(StandardToken::NUMBER, 42.0, "42", 0, 0)));
    ASTPrinter printer;
    
    Value result = _42->accept(printer);
    
    ASSERT_TRUE(result.isCustom());
    EXPECT_STREQ(result.getCustom()->to_str(), "42");
}

TEST_F(ASTPrinterFormatTest, SingleVariable) {
    ASTNodePtr _y(new VariableNode(Token(StandardToken::IDENTIFIER, 0, "y", 0, 0)));
    ASTPrinter printer;
    
    Value result = _y->accept(printer);
    
    ASSERT_TRUE(result.isCustom());
    EXPECT_STREQ(result.getCustom()->to_str(), "y");
}


//  --------------------
//      Lifecycle
//  --------------------

TEST_F(ASTPrinterLifecycleTest, DestroysCorrectly) {
    ASTPrinter* printer = new ASTPrinter();
    printer->destroy(); 
}
