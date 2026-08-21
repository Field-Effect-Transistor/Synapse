//  /tests/core_tests/test_environment_manager.cpp
#include <gtest/gtest.h>
#include "synapse/EnvironmentManager.hpp"
#include "synapse/Module.hpp"
#include "synapse/ExecutionContext.hpp"

using namespace Synapse;

class EnvironmentManagerTest : public ::testing::Test {};

using EnvironmentManagerLifecycleTest   = EnvironmentManagerTest;
using EnvironmentManagerModulesTest     = EnvironmentManagerTest;
using EnvironmentManagerSessionsTest    = EnvironmentManagerTest;
using EnvironmentManagerMoveTest        = EnvironmentManagerTest;

//  --------------------
//      Lifecycle
//  --------------------

TEST_F(EnvironmentManagerLifecycleTest, GlobalModuleIsCreatedByDefault) {
    EnvironmentManager env;
    Module* global_mod = env.getGlobalModule();
    ASSERT_NE(global_mod, nullptr);
    EXPECT_EQ(global_mod->getName(), "Global");
}

//  --------------------
//      Plugin Modules
//  --------------------

TEST_F(EnvironmentManagerModulesTest, CreatesAndRetrievesPluginModules) {
    EnvironmentManager env;
    
    Module* p_mod = env.createPluginModule("test_plugin");
    ASSERT_NE(p_mod, nullptr);
    EXPECT_EQ(p_mod->getName(), "test_plugin");

    Module* retrieved = env.getPluginModule("test_plugin");
    EXPECT_EQ(retrieved, p_mod);
}

TEST_F(EnvironmentManagerModulesTest, ReturnsNullForUnknownPluginModule) {
    EnvironmentManager env;
    Module* retrieved = env.getPluginModule("ghost_plugin");
    EXPECT_EQ(retrieved, nullptr);
}

//  --------------------
//      Session Contexts
//  --------------------

TEST_F(EnvironmentManagerSessionsTest, CreatesAndDeletesSessionContexts) {
    EnvironmentManager env;

    ExecutionContext* ctx1 = env.createSessionContext();
    ExecutionContext* ctx2 = env.createSessionContext();

    ASSERT_NE(ctx1, nullptr);
    ASSERT_NE(ctx2, nullptr);
    EXPECT_NE(ctx1, ctx2);

    env.deleteSessionContext(ctx1);
    
    ExecutionContext fake_ctx;
    env.deleteSessionContext(&fake_ctx);
    
    env.deleteSessionContext(ctx2);
}

//  --------------------
//      Move Semantics
//  --------------------

TEST_F(EnvironmentManagerMoveTest, MoveConstructorTransfersOwnership) {
    EnvironmentManager original;
    original.createPluginModule("plugin_a");
    ExecutionContext* ctx = original.createSessionContext();
    
    EnvironmentManager moved(std::move(original));
    
    EXPECT_NE(moved.getPluginModule("plugin_a"), nullptr);
    moved.deleteSessionContext(ctx);
}

TEST_F(EnvironmentManagerMoveTest, MoveAssignmentTransfersOwnership) {
    EnvironmentManager original;
    original.createPluginModule("plugin_b");
    
    EnvironmentManager target;
    target.createPluginModule("plugin_c");

    target = std::move(original);
    
    EXPECT_NE(target.getPluginModule("plugin_b"), nullptr);
    EXPECT_EQ(target.getPluginModule("plugin_c"), nullptr);
}
