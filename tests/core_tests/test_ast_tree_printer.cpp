//  /tests/core_tests/test_ast_tree_printer.cpp
#include <gtest/gtest.h>
#include "ASTTreePrinter.hpp"
#include "ASTNode.hpp"

using namespace Hermes;

TEST(ASTStructPrinterTest, PrintsTreeStructureCorrectly) {
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

    ASTTreePrinter printer;
    _root->accept(printer);

    std::string expected_output = R"(*
├── +
│   ├── -
│   │   └── 5
│   └── 3
└── max
    ├── 2
    └── x
)";

    EXPECT_EQ(printer.result(), expected_output);
}
