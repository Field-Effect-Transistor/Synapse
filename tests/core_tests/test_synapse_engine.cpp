// /tests/core_tests/test_synapse_engine.cpp
#include <gtest/gtest.h>
#include "synapse/SynapseEngine.hpp"

using namespace Synapse;

//  Manual plugin load
namespace Synapse::Plugin {
    extern "C" IPlugin* create_stdlib_plugin();
}

//  ------------
//      MOCKS
//  ------------

namespace {

    class AddOneCallable final : public ICallable {
    public:
        void destroy() override { delete this; }

        Value operator()(const Vector<Value>& args) override {
            return Value(args[0].getNumber() + 1.0);
        }

        explicit operator bool() const override { return true; }
        size_t arity() const override { return 1; }
    };

    bool containsFullName(const Vector<ToolInfo>& tools, const std::string& full_name) {
        for (const auto& t : tools) {
            if (t.getFullName() == full_name) return true;
        }
        return false;
    }

    bool containsPluginName(const Vector<PluginInfo>& plugins, const std::string& name) {
        for (const auto& p : plugins) {
            if (p.name == name) return true;
        }
        return false;
    }

    bool containsSessionName(const Vector<std::string>& names, const std::string& name) {
        for (const auto& n : names) {
            if (n == name) return true;
        }
        return false;
    }

}   //  namespace

class SynapseEngineTest : public ::testing::Test {
protected:
    SynapseEngine engine;
    Calculator::Recipe std_recipe;

    void SetUp() override {
        engine.loadPlugin(IPlugin::Ptr(Plugin::create_stdlib_plugin()));

        std_recipe.lexer = "stdlib.Standard Lexer";
        std_recipe.parser = "stdlib.Standard Parser";
        std_recipe.evaluator = "stdlib.Math Evaluator";
    }
};

using SynapseSessionManagement  = SynapseEngineTest;
using SynapseEngineMath         = SynapseEngineTest;
using SynapsePluginManagements  = SynapseEngineTest;
using SynapseEngineLifecycle    = SynapseEngineTest;


//  ------------------------------------
//      SYNAPSE SESSIONS MANAGEMENT
//  ------------------------------------

TEST_F(SynapseSessionManagement, GetActiveSessionNameReturnsEmptyWhenNoSession) {
    EXPECT_STREQ(engine.getActiveSessionName(), "");
}

TEST_F(SynapseSessionManagement, CreatesAndSwitchesSessions) {
    EXPECT_FALSE(engine.hasSession("main"));

    engine.createSession("main", std_recipe);
    EXPECT_TRUE(engine.hasSession("main"));
    EXPECT_STREQ(engine.getActiveSessionName(), "main");

    engine.createSession("secondary", std_recipe);
    EXPECT_TRUE(engine.hasSession("secondary"));
    EXPECT_STREQ(engine.getActiveSessionName(), "main"); // Активною лишається перша

    engine.switchSession("secondary");
    EXPECT_STREQ(engine.getActiveSessionName(), "secondary");
}

TEST_F(SynapseSessionManagement, ThrowsOnInvalidSessionOperations) {
    EXPECT_THROW(engine.switchSession("ghost_session"), std::runtime_error);
    EXPECT_THROW(engine.deleteSession("ghost_session"), std::runtime_error);
}

TEST_F(SynapseSessionManagement, ThrowsOnDuplicateSessionCreation) {
    engine.createSession("main", std_recipe);
    // Спроба створити сесію з існуючим іменем має викидати помилку
    EXPECT_THROW(engine.createSession("main", std_recipe), std::runtime_error);
}

TEST_F(SynapseSessionManagement, DeleteSessionRemovesSessionAndClearsActiveIfNeeded) {
    engine.createSession("main", std_recipe);
    engine.createSession("secondary", std_recipe);
    
    // Видалення неактивної сесії
    engine.deleteSession("secondary");
    EXPECT_FALSE(engine.hasSession("secondary"));
    EXPECT_STREQ(engine.getActiveSessionName(), "main"); // 'main' досі активна

    // Видалення активної сесії
    engine.deleteSession("main");
    EXPECT_FALSE(engine.hasSession("main"));
    EXPECT_STREQ(engine.getActiveSessionName(), ""); // Активної більше немає
}

TEST_F(SynapseSessionManagement, GetAvailableSessionsReturnsAllSessionNames) {
    engine.createSession("main", std_recipe);
    engine.createSession("secondary", std_recipe);

    auto names = engine.getAvailableSessions();
    ASSERT_EQ(names.size(), 2u);
    EXPECT_TRUE(containsSessionName(names, "main"));
    EXPECT_TRUE(containsSessionName(names, "secondary"));
}

//  ----------------------------
//      SYNAPSE ENGINE MATH
//  ----------------------------

TEST_F(SynapseEngineMath, EvaluatesCodeWithoutActiveSessionThrows) {
    EXPECT_THROW(engine.evaluate("2 + 2"), std::runtime_error);
}

TEST_F(SynapseEngineMath, EvaluatesBasicMathInSession) {
    engine.createSession("main", std_recipe);

    EXPECT_DOUBLE_EQ(engine.evaluate("10 + 5 * 2").getNumber(), 20.0);
    EXPECT_NEAR(engine.evaluate("pi * 2").getNumber(), 6.28318, 0.0001);
    EXPECT_DOUBLE_EQ(engine.evaluate("max(10, 50)").getNumber(), 50.0);
}

TEST_F(SynapseEngineMath, ThrowsOnInvalidMathExpressions) {
    engine.createSession("main", std_recipe);

    // Синтаксична помилка
    EXPECT_THROW(engine.evaluate("10 + * 2"), std::runtime_error);
    // Невідома функція
    EXPECT_THROW(engine.evaluate("unknown_function(42)"), std::runtime_error);
}

TEST_F(SynapseEngineMath, MagicAnsVariableSavesLastResult) {
    engine.createSession("main", std_recipe);

    EXPECT_DOUBLE_EQ(engine.evaluate("10 + 5").getNumber(), 15.0);
    EXPECT_DOUBLE_EQ(engine.evaluate("ans * 2").getNumber(), 30.0);
    EXPECT_DOUBLE_EQ(engine.evaluate("ans - 5").getNumber(), 25.0);
}

TEST_F(SynapseEngineMath, SessionsAreIsolated) {
    engine.createSession("tab1", std_recipe);
    engine.createSession("tab2", std_recipe);

    engine.switchSession("tab1");
    engine.defineGlobalVariable("global_x", Value(100.0));
    EXPECT_DOUBLE_EQ(engine.evaluate("global_x + 10").getNumber(), 110.0);

    engine.switchSession("tab2");
    EXPECT_DOUBLE_EQ(engine.evaluate("global_x * 2").getNumber(), 200.0);
}

TEST_F(SynapseEngineMath, FormatASTRunsAlternateVisitorWithoutThrowing) {
    engine.loadPluginFromFile(PRINTER_LIB_PATH);
    engine.createSession("main", std_recipe);

    EXPECT_NO_THROW({
        Value result = engine.formatAST("PrinterLib.ASTPrinter", "1 + 2");
        (void)result;
    });
}


//  ------------------------------------
//      SYNAPSE PLUGIN MANAGEMENTS
//  ------------------------------------

TEST_F(SynapsePluginManagements, ThrowsOnLoadingInvalidPluginFile) {
    EXPECT_THROW(engine.loadPluginFromFile("invalid_path/non_existent_plugin.dll"), std::runtime_error);
}

TEST_F(SynapsePluginManagements, LoadsPluginFromFileAndRegistersModule) {
    engine.loadPluginFromFile(PRINTER_LIB_PATH);

    auto plugins = engine.getLoadedPlugins();
    EXPECT_TRUE(containsPluginName(plugins, "PrinterLib"));

    auto visitors = engine.getAvailableVisitors();
    EXPECT_TRUE(containsFullName(visitors, "PrinterLib.ASTPrinter"));
    EXPECT_TRUE(containsFullName(visitors, "PrinterLib.ASTTreePrinter"));
}

TEST_F(SynapsePluginManagements, DefinesGlobalFunctionAndCallsItInSession) {
    engine.defineGlobalFunction("addOne", ICallable::Ptr(new AddOneCallable()));

    engine.createSession("main", std_recipe);
    Value result = engine.evaluate("addOne(41)");
    EXPECT_DOUBLE_EQ(result.getNumber(), 42.0);
}

TEST_F(SynapsePluginManagements, DefinesGlobalVariableSuccessfully) {
    engine.defineGlobalVariable("CUSTOM_PI", Value(3.14));
    engine.createSession("main", std_recipe);
    
    EXPECT_DOUBLE_EQ(engine.evaluate("CUSTOM_PI * 2").getNumber(), 6.28);
}

TEST_F(SynapsePluginManagements, DiscoveryApiReturnsLoadedPluginsAndTools) {
    auto plugins = engine.getLoadedPlugins();
    ASSERT_GE(plugins.size(), 1u); // Має бути мінімум stdlib
    EXPECT_TRUE(containsPluginName(plugins, "stdlib"));

    auto lexers = engine.getAvailableLexers();
    EXPECT_TRUE(containsFullName(lexers, "stdlib.Standard Lexer"));

    auto parsers = engine.getAvailableParsers();
    EXPECT_TRUE(containsFullName(parsers, "stdlib.Standard Parser"));

    EXPECT_NO_THROW(engine.getAvailableVisitors());
}

//  --------------------------------
//      SYNAPSE ENGINE LIFECYCLE
//  --------------------------------

TEST_F(SynapseEngineLifecycle, MoveConstructorTransfersState) {
    engine.createSession("main", std_recipe);
    ASSERT_STREQ(engine.getActiveSessionName(), "main");

    SynapseEngine moved(std::move(engine));

    EXPECT_TRUE(moved.hasSession("main"));
    EXPECT_STREQ(moved.getActiveSessionName(), "main");

    Value result = moved.evaluate("2 + 2");
    EXPECT_DOUBLE_EQ(result.getNumber(), 4.0);
}

TEST_F(SynapseEngineLifecycle, MoveAssignmentTransfersState) {
    engine.createSession("main", std_recipe);

    SynapseEngine other;
    other = std::move(engine);

    EXPECT_TRUE(other.hasSession("main"));
    EXPECT_STREQ(other.getActiveSessionName(), "main");

    Value result = other.evaluate("3 + 3");
    EXPECT_DOUBLE_EQ(result.getNumber(), 6.0);
}
