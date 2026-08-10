// /tests/core_tests/test_math_evaluator.cpp
#include <gtest/gtest.h>
#include <sstream>

#include "synapse/ContextManager.hpp"
#include "synapse/Context.hpp"
#include "synapse/Callable.hpp"
#include "internal/Lexer.hpp"
#include "internal/Parser.hpp"
#include "internal/MathEvaluator.hpp"
#include "synapse/Exceptions.hpp"

using namespace Synapse;
using namespace Synapse::Internal;

class MathEvaluatorTest : public ::testing::Test {
protected:
    ContextManager manager;
    Context* global_ctx;

    void SetUp() override {
        global_ctx = manager.getGlobalScope();
    }

    Value eval_code(const std::string& code, Context* ctx = nullptr) {
        if (!ctx) ctx = global_ctx;

        std::istringstream stream(code);
        Lexer lexer(stream);
        
        Vector<Token> tokens;
        while (true) {
            Token t = lexer.getNextToken();
            bool is_eof = (t.type == StandardToken::END_OF_FILE);
            
            if (t.type != StandardToken::COMMENT) {
                tokens.push_back(std::move(t));
            }
            
            if (is_eof) break;
        }

        Parser parser;
        ASTNodePtr ast = parser.parse(tokens);

        MathEvaluator evaluator(ctx);
        return ast->accept(evaluator);
    }
};

// =====================
//      BASIC MATH
// =====================
TEST_F(MathEvaluatorTest, EvaluatesBasicMathAndPrecedence) {
    EXPECT_DOUBLE_EQ(eval_code("2 + 2 * 2").getNumber(), 6.0);
    EXPECT_DOUBLE_EQ(eval_code("(2 + 2) * 2").getNumber(), 8.0);
    EXPECT_DOUBLE_EQ(eval_code("10 / 2 - 1").getNumber(), 4.0);
    EXPECT_DOUBLE_EQ(eval_code("2 ^ 3").getNumber(), 8.0);
    EXPECT_DOUBLE_EQ(eval_code("10 mod 3").getNumber(), 1.0);
    EXPECT_DOUBLE_EQ(eval_code("-5 + 10").getNumber(), 5.0);
    EXPECT_DOUBLE_EQ(eval_code("50%").getNumber(), 0.5); // Відсоток
}

// =============
//      VARS
// =============
TEST_F(MathEvaluatorTest, EvaluatesVariablesFromContext) {
    global_ctx->defineVariable("x", Value(10.0));
    global_ctx->defineVariable("y", Value(5.0));

    EXPECT_DOUBLE_EQ(eval_code("x * y + 2").getNumber(), 52.0);
}

// =================
//      FUNCS
// =================
TEST_F(MathEvaluatorTest, EvaluatesFunctionCalls) {
    auto my_max = make_callable(2, [](const Vector<Value>& args) {
        double a = args[0].getNumber();
        double b = args[1].getNumber();
        return Value(a > b ? a : b);
    });

    global_ctx->defineFunction("max", std::move(my_max));
    global_ctx->defineVariable("x", Value(5.0));

    EXPECT_DOUBLE_EQ(eval_code("max(x, 10) * 2").getNumber(), 20.0);
    
    EXPECT_DOUBLE_EQ(eval_code("max(1, max(2, 3))").getNumber(), 3.0);
}

// =============================
//      RUNTIME EXCEPTIONS
// =============================
TEST_F(MathEvaluatorTest, ThrowsOnRuntimeErrors) {
    // Ділення на нуль
    EXPECT_THROW(eval_code("10 / 0"), RuntimeError);
    EXPECT_THROW(eval_code("10 mod 0"), RuntimeError);

    // Невідома змінна
    EXPECT_THROW(eval_code("unknown_var + 1"), std::runtime_error);

    // Невідома функція
    EXPECT_THROW(eval_code("unknown_func(10)"), std::runtime_error);

    // Погана кількість аргументів (Arity)
    auto my_sqrt = make_callable(1, [](const Vector<Value>& args) { return Value(1.0); });
    global_ctx->defineFunction("sqrt", std::move(my_sqrt));

    EXPECT_THROW(eval_code("sqrt(10, 20)"), ArityMismatchError);
}
