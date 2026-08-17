// /tests/core_tests/test_calculator.cpp
#include <gtest/gtest.h>

#include "synapse/Calculator.hpp"
#include "synapse/CalculatorBuilder.hpp"
#include "synapse/PluginRegistry.hpp"
#include "synapse/ContextManager.hpp"
#include "synapse/Context.hpp"

using namespace Synapse;

namespace Synapse::Plugin {
    extern "C" IPlugin* create_stdlib_plugin();
}

class CalculatorIntegrationTest : public ::testing::Test {
protected:
    PluginRegistry registry;
    ContextManager context_manager;
    Context* global_context;

    void SetUp() override {
        global_context = context_manager.getGlobalScope();
        
        IPlugin::Ptr stdlib(Plugin::create_stdlib_plugin());
        registry.loadPlugin(std::move(stdlib));
        
        registry.fillContext(global_context);
    }
};

TEST_F(CalculatorIntegrationTest, BuilderThrowsOnMissingLexer) {
    CalculatorBuilder builder(&registry);
    builder.useParser("stdlib.Standard Parser")
           .useEvaluator("stdlib.Math Evaluator");

    EXPECT_THROW(builder.build(), std::runtime_error);
}

TEST_F(CalculatorIntegrationTest, BuilderCreatesValidCalculator) {
    CalculatorBuilder builder(&registry);
    builder.useLexer("stdlib.Standard Lexer")
           .useParser("stdlib.Standard Parser")
           .useEvaluator("stdlib.Math Evaluator");

    Calculator calc = builder.build(); 
}

TEST_F(CalculatorIntegrationTest, EvaluatesComplexMathematicalExpression) {
    // Збираємо калькулятор
    Calculator calc = CalculatorBuilder(&registry)
        .useLexer("stdlib.Standard Lexer")
        .useParser("stdlib.Standard Parser")
        .useEvaluator("stdlib.Math Evaluator")
        .build();

    // 1. Базова математика
    Value res1 = calc.evaluate("10 + 2 * 5", global_context);
    EXPECT_TRUE(res1.isNumber());
    EXPECT_DOUBLE_EQ(res1.getNumber(), 20.0);

    // 2. Використання констант з плагіна (pi)
    Value res2 = calc.evaluate("pi * 2", global_context);
    EXPECT_DOUBLE_EQ(res2.getNumber(), 6.283185307179586);

    // 3. Використання функцій з плагіна (max, sin)
    Value res3 = calc.evaluate("max(10, 20) + sin(0)", global_context);
    EXPECT_DOUBLE_EQ(res3.getNumber(), 20.0);
}

TEST_F(CalculatorIntegrationTest, EvaluatesWithLocalVariablesInUserScope) {
    // Створюємо "Користувацький" Scope, батьком якого є Global
    Context* user_scope = context_manager.createScope(global_context);
    user_scope->defineVariable("my_var", Value(42.0));

    // Створюємо Калькулятор, який працює ТІЛЬКИ в User Scope
    Calculator calc = CalculatorBuilder(&registry)
        .useLexer("stdlib.Standard Lexer")
        .useParser("stdlib.Standard Parser")
        .useEvaluator("stdlib.Math Evaluator")
        .build();

    // Перевіряємо, чи калькулятор бачить і локальну змінну, і глобальну функцію
    Value res = calc.evaluate("max(my_var, 10)", user_scope);
    EXPECT_DOUBLE_EQ(res.getNumber(), 42.0);
}
