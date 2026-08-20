// /tests/core_tests/test_synapse_engine.cpp
#include <gtest/gtest.h>
#include "synapse/SynapseEngine.hpp"

using namespace Synapse;

// Оголошуємо фабрику StdLibPlugin, щоб завантажити її вручну для тестів
namespace Synapse::Plugin {
    extern "C" IPlugin* create_stdlib_plugin();
}

class SynapseEngineTest : public ::testing::Test {
protected:
    SynapseEngine engine;
    Calculator::Recipe std_recipe;

    void SetUp() override {
        // 1. Вантажимо стандартну математику
        engine.loadPlugin(IPlugin::Ptr(Plugin::create_stdlib_plugin()));

        // 2. Створюємо рецепт для тестування (використовуємо інструменти зі StdLib)
        std_recipe.lexer = "stdlib.Standard Lexer";
        std_recipe.parser = "stdlib.Standard Parser";
        std_recipe.evaluator = "stdlib.Math Evaluator";
    }
};


// ==========================================
// БЛОК 1: Сесії (Створення та Управління)
// ==========================================

TEST_F(SynapseEngineTest, CreatesAndSwitchesSessions) {
    EXPECT_FALSE(engine.hasSession("main"));

    // Створюємо сесію
    engine.createSession("main", std_recipe);
    EXPECT_TRUE(engine.hasSession("main"));
    
    // Оскільки вона перша, вона має стати активною
    EXPECT_STREQ(engine.getActiveSessionName(), "main");

    // Створюємо другу сесію
    engine.createSession("secondary", std_recipe);
    EXPECT_TRUE(engine.hasSession("secondary"));

    // Активною все ще має залишатися "main"
    EXPECT_STREQ(engine.getActiveSessionName(), "main");

    // Перемикаємося на "secondary"
    engine.switchSession("secondary");
    EXPECT_STREQ(engine.getActiveSessionName(), "secondary");
}

TEST_F(SynapseEngineTest, ThrowsOnInvalidSessionOperations) {
    // Перемикання на неіснуючу
    EXPECT_THROW(engine.switchSession("ghost"), std::runtime_error);

    // Подвійне створення з тим самим ім'ям
    engine.createSession("main", std_recipe);
    EXPECT_THROW(engine.createSession("main", std_recipe), std::runtime_error);

    // Видалення неіснуючої
    EXPECT_THROW(engine.deleteSession("ghost"), std::runtime_error);
}

TEST_F(SynapseEngineTest, EvaluatesCodeWithoutActiveSessionThrows) {
    // Намагаємося рахувати без створення сесії
    EXPECT_THROW(engine.evaluate("2 + 2"), std::runtime_error);
}


// ==========================================
// БЛОК 2: Виконання математики та Ізоляція
// ==========================================

TEST_F(SynapseEngineTest, EvaluatesBasicMathInSession) {
    engine.createSession("main", std_recipe);
    
    // Проста математика
    Value res1 = engine.evaluate("10 + 5 * 2");
    EXPECT_DOUBLE_EQ(res1.getNumber(), 20.0);

    // Використання констант зі StdLibPlugin
    Value res2 = engine.evaluate("pi * 2");
    EXPECT_NEAR(res2.getNumber(), 6.28318, 0.0001);

    // Використання функцій зі StdLibPlugin
    Value res3 = engine.evaluate("max(10, 50)");
    EXPECT_DOUBLE_EQ(res3.getNumber(), 50.0);
}

TEST_F(SynapseEngineTest, SessionsAreIsolated) {
    engine.createSession("tab1", std_recipe);
    engine.createSession("tab2", std_recipe);

    // Працюємо в tab1
    engine.switchSession("tab1");
    // Створюємо змінну x (оскільки evaluator не має assign_expr, ми мусимо додати її руками в global,
    // АЛЕ зараз ми просто перевіряємо, чи впаде пошук у tab2)
    engine.defineGlobalVariable("global_x", Value(100.0)); // Спільно для всіх

    // ... Оскільки у нас ще немає оператора присвоювання у синтаксисі (типу x = 5),
    // повну ізоляцію змінних користувача ми перевіримо пізніше, коли додамо парсинг Assignment.
    // Поки перевіряємо доступ до глобальних.
    
    engine.switchSession("tab1");
    EXPECT_DOUBLE_EQ(engine.evaluate("global_x + 10").getNumber(), 110.0);
    
    engine.switchSession("tab2");
    EXPECT_DOUBLE_EQ(engine.evaluate("global_x * 2").getNumber(), 200.0);
}


// ==========================================
// БЛОК 3: Магічна змінна `ans`
// ==========================================

TEST_F(SynapseEngineTest, MagicAnsVariableSavesLastResult) {
    engine.createSession("main", std_recipe);

    // Перше обчислення
    Value res1 = engine.evaluate("10 + 5");
    EXPECT_DOUBLE_EQ(res1.getNumber(), 15.0);

    // Друге обчислення використовує 'ans'
    Value res2 = engine.evaluate("ans * 2");
    EXPECT_DOUBLE_EQ(res2.getNumber(), 30.0);

    // Третє обчислення знову оновлює 'ans'
    Value res3 = engine.evaluate("ans - 5");
    EXPECT_DOUBLE_EQ(res3.getNumber(), 25.0);
}
