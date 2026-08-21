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

    //  ------------
    //      MOCKS
    //  ------------

    struct DummyOptimizer : public IVisitor {
        void destroy() override { delete this; }
        void setContext(ExecutionContext*) override {};
        const Role getRole() const override { return Role::Preprocessor; }
        Value visit(LiteralNode&) override { return Value(); }
        Value visit(VariableNode&) override { return Value(); }
        Value visit(BinaryNode&) override { return Value(); }
        Value visit(UnaryNode&) override { return Value(); }
        Value visit(FunctionNode&) override { return Value(); }
    };  //  struct  DummyOptimizer
    
    IVisitor* create_dummy_optimizer(ExecutionContext*) { return new DummyOptimizer(); }
    struct DummyProducer : public IVisitor {
        void destroy() override { delete this; }
        void setContext(ExecutionContext*) override {};
        const Role getRole() const override { return Role::Producer; }
        Value visit(LiteralNode&) override { return Value(99.0); }
        Value visit(VariableNode&) override { return Value(99.0); }
        Value visit(BinaryNode&) override { return Value(99.0); }
        Value visit(UnaryNode&) override { return Value(99.0); }
        Value visit(FunctionNode&) override { return Value(99.0); }
    };
    
    IVisitor* create_dummy_producer(ExecutionContext*) { return new DummyProducer(); }

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
            m.parsers.push_back({"NullParser", "Returns null AST", &create_dummy_parser_null});
            
            m.visitors.push_back({"DummyOpt", "Does nothing", IVisitor::Role::Preprocessor, &create_dummy_optimizer});
            m.visitors.push_back({"DummyProducer", "Returns 99", IVisitor::Role::Producer, &create_dummy_producer});
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

//  -----------------------------------
//      Role Validation (Warnings)
//  -----------------------------------

TEST_F(CalculatorEvaluateTest, TriggersWarningWhenProducerIsUsedAsOptimizer) {
    bool warning_triggered = false;
    std::string warning_message = "";

    auto callback = [&](const std::string& msg) {
        warning_triggered = true;
        warning_message = msg;
    };

    Calculator::Recipe bad_opt_recipe = std_recipe;
    bad_opt_recipe.optimizers.push_back("helper.DummyProducer");

    Calculator calc(&registry, bad_opt_recipe, callback);
    calc.evaluate("10 + 20", global_context);

    EXPECT_TRUE(warning_triggered);
    EXPECT_NE(warning_message.find("has role Producer, but is used as a Preprocessor"), std::string::npos);
}

TEST_F(CalculatorEvaluateTest, TriggersWarningWhenPreprocessorIsUsedAsEvaluator) {
    bool warning_triggered = false;
    std::string warning_message = "";

    auto callback = [&](const std::string& msg) {
        warning_triggered = true;
        warning_message = msg;
    };

    Calculator::Recipe bad_eval_recipe = std_recipe;
    bad_eval_recipe.evaluator = "helper.DummyOpt";

    Calculator calc(&registry, bad_eval_recipe, callback);
    calc.evaluate("10 + 20", global_context);

    EXPECT_TRUE(warning_triggered);
    EXPECT_NE(warning_message.find("has role Preprocessor, but is used as the final evaluator"), std::string::npos);
}

TEST_F(CalculatorEvaluateTest, NoWarningsForCorrectRoles) {
    bool warning_triggered = false;
    auto callback = [&](const std::string&) { warning_triggered = true; };

    Calculator::Recipe good_recipe = std_recipe;
    good_recipe.optimizers.push_back("helper.DummyOpt");

    Calculator calc(&registry, good_recipe, callback);
    calc.evaluate("10 + 20", global_context);

    EXPECT_FALSE(warning_triggered);
}
