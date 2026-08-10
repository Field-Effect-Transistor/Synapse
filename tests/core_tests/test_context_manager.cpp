// /tests/core_tests/test_context_manager.cpp
#include <gtest/gtest.h>
#include "synapse/ContextManager.hpp"
#include "synapse/Context.hpp"

using namespace Synapse;

class ContextManagerTest : public ::testing::Test {
protected:
    Value num(double v) { return Value(v); }
};

// =================
//      BASIC
// =================
TEST_F(ContextManagerTest, GlobalScopeExistsOnCreation) {
    ContextManager manager;
    Context* global = manager.getGlobalScope();
    
    ASSERT_NE(global, nullptr);

    global->defineVariable("pi", num(3.14));
    EXPECT_DOUBLE_EQ(global->getVariable("pi").getNumber(), 3.14);
}

// =================================
//      HIERARCHY n SHADOWING
// =================================
TEST_F(ContextManagerTest, ScopeChainAndShadowingWorkCorrectly) {
    ContextManager manager;
    Context* global = manager.getGlobalScope();
    
    global->defineVariable("g", num(9.8));
    global->defineVariable("e", num(2.71));

    Context* plugin = manager.createScope(global);
    plugin->defineVariable("g", num(10.0));
    plugin->defineVariable("friction", num(0.5));

    Context* user = manager.createScope(plugin);
    user->defineVariable("my_var", num(42.0));

    // Власна змінна
    EXPECT_DOUBLE_EQ(user->getVariable("my_var").getNumber(), 42.0);
    // Змінна з плагіна (на 1 рівень вище)
    EXPECT_DOUBLE_EQ(user->getVariable("friction").getNumber(), 0.5);
    // Перекрита змінна (повинна взятись з плагіна, а не з Global)
    EXPECT_DOUBLE_EQ(user->getVariable("g").getNumber(), 10.0);
    // Глибока змінна (на 2 рівні вище)
    EXPECT_DOUBLE_EQ(user->getVariable("e").getNumber(), 2.71);

    // --- Перевірки для Global Scope ---
    // Глобальний Scope не має знати про змінні дітей
    EXPECT_THROW(global->getVariable("friction"), std::runtime_error);
    // Його власна змінна має залишитись незмінною
    EXPECT_DOUBLE_EQ(global->getVariable("g").getNumber(), 9.8);
}

// =================
//      MEMORY
// =================
TEST_F(ContextManagerTest, ClearDestroysDataAndRecreatesGlobalScope) {
    ContextManager manager;
    Context* old_global = manager.getGlobalScope();
    old_global->defineVariable("x", num(100.0));

    manager.clear();

    Context* new_global = manager.getGlobalScope();
    
    ASSERT_NE(new_global, nullptr);
    EXPECT_THROW(new_global->getVariable("x"), std::runtime_error);
}

// =============================
//      PARALLEL BRANCHES
// =============================
TEST_F(ContextManagerTest, ParallelBranchesAreIsolated) {
    ContextManager manager;
    Context* global = manager.getGlobalScope();
    global->defineVariable("base", num(10.0));

    Context* user1 = manager.createScope(global);
    Context* user2 = manager.createScope(global);

    user1->defineVariable("local", num(100.0));
    user2->defineVariable("local", num(200.0));

    EXPECT_DOUBLE_EQ(user1->getVariable("base").getNumber(), 10.0);
    EXPECT_DOUBLE_EQ(user2->getVariable("base").getNumber(), 10.0);

    EXPECT_DOUBLE_EQ(user1->getVariable("local").getNumber(), 100.0);
    EXPECT_DOUBLE_EQ(user2->getVariable("local").getNumber(), 200.0);

    user2->defineVariable("secret", num(999.0));
    EXPECT_THROW(user1->getVariable("secret"), std::runtime_error);
}

// =========================
//      MOVE SEMANTICS
// =========================
TEST_F(ContextManagerTest, MoveSemanticsTransferOwnershipCorrectly) {
    ContextManager manager1;
    manager1.getGlobalScope()->defineVariable("x", num(55.0));

    ContextManager manager2(std::move(manager1));

    Context* global2 = manager2.getGlobalScope();
    ASSERT_NE(global2, nullptr);
    EXPECT_DOUBLE_EQ(global2->getVariable("x").getNumber(), 55.0);

    EXPECT_EQ(manager1.getGlobalScope(), nullptr);
}
