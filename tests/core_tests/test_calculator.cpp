//  /tests/core_tests/test_calculator.cpp
#include <gtest/gtest.h>

#include "synapse/Calculator.hpp"
#include "synapse/PluginRegistry.hpp"
#include "synapse/ExecutionContext.hpp"
#include "synapse/EnvironmentManager.hpp"
#include "synapse/Module.hpp"

using namespace Synapse;

namespace Synapse::Plugin {
    extern "C" IPlugin* create_stdlib_plugin();
}   //  namespace   Synapse::Plugin

namespace {

    struct DummyOptimizer : public IVisitor {
        void destroy() override { delete this; }
        Value visit(LiteralNode&) override { return Value(); }
        Value visit(VariableNode&) override { return Value(); }
        Value visit(BinaryNode&) override { return Value(); }
        Value visit(UnaryNode&) override { return Value(); }
        Value visit(FunctionNode&) override { return Value(); }
    };  //  struct  DummyOptimizer
    
    IVisitor* create_dummy_optimizer() { return new DummyOptimizer(); }
    
    struct DummyParserNull : public IParser {
        void destroy() override { delete this; }
        IASTNode::Ptr parse(const Vector<Token>&) override { return IASTNode::Ptr(nullptr); }
    };  //  struct  DummyParserNull
    
    IParser* create_dummy_parser_null() { return new DummyParserNull(); }

    class TestHelperPlugin : public IPlugin {
    public:
        void destroy() override { delete this; }
        const char* getName() const override { return "helper"; }
        const char* getVersion() const override { return "1.0"; }
        const char* getDescription() const override { return "Helper Plugin"; }
        
        PluginManifest getManifest() const override {
            PluginManifest m;
            m.simple_visitors.push_back({"DummyOpt", "Does nothing", &create_dummy_optimizer});
            m.parsers.push_back({"NullParser", "Returns null AST", &create_dummy_parser_null});
            return m;
        }
    };  //  class   TestHelperPlugin

}   //  namespace

class CalculatorIntegrationTest : public ::testing::Test {
protected:
    PluginRegistry registry;
    EnvironmentManager env_manager;
    ExecutionContext* global_context;
    
    Calculator::Recipe std_recipe;

    void SetUp() override {
        global_context = env_manager.createSessionContext();
        
        IPlugin::Ptr stdlib(Plugin::create_stdlib_plugin());
        registry.loadPlugin(std::move(stdlib));
        
        registry.loadPlugin(IPlugin::Ptr(new TestHelperPlugin()));

        Module* mod = env_manager.createPluginModule("stdlib");
        registry.fillModule(*mod, "stdlib");
        global_context->importModule(mod);
        
        std_recipe = {
            "stdlib.Standard Lexer",
            "stdlib.Standard Parser",
            "stdlib.Math Evaluator"
        };
    }
};  //  class   CalculatorIntegrationTest

using CalculatorInitTest        = CalculatorIntegrationTest;
using CalculatorEvaluateTest    = CalculatorIntegrationTest;

//  ------------------------
//      Initialization
//  ------------------------

TEST_F(CalculatorInitTest, ConstructorThrowsWhenRegistryIsNull) {
    EXPECT_THROW(Calculator(nullptr, std_recipe), std::runtime_error);
}

TEST_F(CalculatorInitTest, ConstructorThrowsOnIncompleteRecipe) {
    Calculator::Recipe missing_lexer   = {"", "stdlib.Standard Parser", "stdlib.Math Evaluator"};
    Calculator::Recipe missing_parser  = {"stdlib.Standard Lexer", "", "stdlib.Math Evaluator"};
    Calculator::Recipe missing_eval    = {"stdlib.Standard Lexer", "stdlib.Standard Parser", ""};

    EXPECT_THROW(Calculator(&registry, missing_lexer), std::runtime_error);
    EXPECT_THROW(Calculator(&registry, missing_parser), std::runtime_error);
    EXPECT_THROW(Calculator(&registry, missing_eval), std::runtime_error);
}

TEST_F(CalculatorInitTest, ConstructorCreatesValidCalculator) {
    EXPECT_NO_THROW({
        Calculator calc(&registry, std_recipe);
    });
}

//  --------------------
//      Evaluation
//  --------------------

TEST_F(CalculatorEvaluateTest, EvaluatesThrowsWhenContextMissing) {
    Calculator calc(&registry, std_recipe);
    EXPECT_THROW(calc.evaluate("1 + 1", nullptr), std::runtime_error);
}

TEST_F(CalculatorEvaluateTest, ReturnsZeroWhenAstIsNull) {
    Calculator::Recipe null_ast_recipe = {
        "stdlib.Standard Lexer",
        "helper.NullParser",
        "stdlib.Math Evaluator"
    };
    
    Calculator calc(&registry, null_ast_recipe);
    Value res = calc.evaluate("10 + 20", global_context);
    EXPECT_DOUBLE_EQ(res.getNumber(), 0.0);
}

TEST_F(CalculatorEvaluateTest, RunsOptimizers) {
    Calculator::Recipe opt_recipe = std_recipe;
    opt_recipe.optimizers.push_back("helper.DummyOpt");

    Calculator calc(&registry, opt_recipe);

    Value res = calc.evaluate("10 + 20", global_context);
    EXPECT_DOUBLE_EQ(res.getNumber(), 30.0);
}

TEST_F(CalculatorEvaluateTest, EvaluatesComplexMathematicalExpression) {
    Calculator calc(&registry, std_recipe);

    Value res1 = calc.evaluate("10 + 2 * 5", global_context);
    EXPECT_TRUE(res1.isNumber());
    EXPECT_DOUBLE_EQ(res1.getNumber(), 20.0);

    Value res2 = calc.evaluate("pi * 2", global_context);
    EXPECT_DOUBLE_EQ(res2.getNumber(), 6.283185307179586);

    Value res3 = calc.evaluate("max(10, 20) + sin(0)", global_context);
    EXPECT_DOUBLE_EQ(res3.getNumber(), 20.0);
}

TEST_F(CalculatorEvaluateTest, EvaluatesWithLocalVariablesInUserScope) {
    ExecutionContext* user_scope = env_manager.createSessionContext();
    user_scope->importModule(env_manager.getPluginModule("stdlib"));
    user_scope->defineVariable("my_var", Value(42.0));

    Calculator calc(&registry, std_recipe);

    Value res = calc.evaluate("max(my_var, 10)", user_scope);
    EXPECT_DOUBLE_EQ(res.getNumber(), 42.0);
}
