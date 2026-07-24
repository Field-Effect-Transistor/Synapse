#include <gtest/gtest.h>
#include "ASTNode.hpp"
#include "ASTPrinter.hpp"

using namespace Hermes;

TEST(ASTTest, TestViaPrinter) {
    ASTNodePtr  _5(new LeafNode(5.)),
                _3(new LeafNode(3.)),
                _2(new LeafNode(2.)),
                _x(new VariableNode(Token(StandardToken::IDENTIFIER, 0, "x", 0, 0)));
    
    ASTNodePtr  _m5(new UnaryNode(Token(StandardToken::SUB, 0, "-", 0, 0), std::move(_5)));

    ASTNodePtr  _m5p3(new BinaryNode(Token(StandardToken::ADD, 0, "+", 0, 0), std::move(_m5), std::move(_3)));

    std::vector<ASTNodePtr> max_args;
    max_args.push_back(std::move(_2));
    max_args.push_back(std::move(_x));

    ASTNodePtr _max2x(new FunctionNode(
        Token(StandardToken::IDENTIFIER, 0, "max", 0, 0), 
        std::move(max_args)
    ));
    ASTNodePtr  _root(new BinaryNode(Token(StandardToken::MUL, 0, "*", 0, 0), std::move(_m5p3), std::move(_max2x)));

    ASTPrinter printer;
    _root->accept(printer);

    EXPECT_EQ(printer.result(), "( * ( + ( - 5.000000 ) 3.000000 ) ( max 2.000000 x ) )");
}

