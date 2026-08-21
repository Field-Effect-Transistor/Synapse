//  /tests/core_tests/test_memory_model.cpp
#include <gtest/gtest.h>
#include "synapse/SymbolTable.hpp"
#include "synapse/Module.hpp"
#include "synapse/ExecutionContext.hpp"
#include "synapse/Value.hpp"
#include "synapse/Callable.hpp"

#include <stdexcept>
#include <string>

using namespace Synapse;
using CallablePtr = ICallable::Ptr;

CallablePtr create_dummy_func(double return_value) {
    return make_callable(0, [return_value](const Vector<Value>&) {
        return Value(return_value);
    });
}

class MemoryModelTest : public ::testing::Test {};

using SymbolTableTest       = MemoryModelTest;
using ModuleTest            = MemoryModelTest;
using ExecutionContextTest  = MemoryModelTest;

//  --------------------
//      SymbolTable
//  --------------------

TEST_F(SymbolTableTest, DefinesAndGetsVariables) {
    SymbolTable table;
    table.defineVariable("x", Value(42.0));
    table.defineVariable("flag", Value(true));
    
    EXPECT_TRUE(table.hasVariable("x"));
    EXPECT_TRUE(table.hasVariable("flag"));
    EXPECT_FALSE(table.hasVariable("y"));
    
    EXPECT_DOUBLE_EQ(table.getVariable("x").getNumber(), 42.0);
    EXPECT_TRUE(table.getVariable("flag").getBool());
}

TEST_F(SymbolTableTest, AssignsExistingVariables) {
    SymbolTable table;
    table.defineVariable("x", Value(10.0));
    
    table.assignVariable("x", Value(99.0));
    EXPECT_DOUBLE_EQ(table.getVariable("x").getNumber(), 99.0);
}

TEST_F(SymbolTableTest, ThrowsOnInvalidVariableOperations) {
    SymbolTable table;
    
    EXPECT_THROW(table.getVariable("ghost"), std::runtime_error);
    EXPECT_THROW(table.assignVariable("ghost", Value(10.0)), std::runtime_error);
    
    table.defineVariable("x", Value(1.0));
    EXPECT_THROW(table.defineVariable("x", Value(2.0)), std::runtime_error);
}

TEST_F(SymbolTableTest, EnforcesConstCorrectness) {
    SymbolTable table;
    table.defineVariable("pi", Value(3.1415), true);
    
    EXPECT_TRUE(table.getVariable("pi").is_constant());
    EXPECT_DOUBLE_EQ(table.getVariable("pi").getNumber(), 3.1415);
    
    EXPECT_THROW(table.assignVariable("pi", Value(3.0)), std::runtime_error);
}

TEST_F(SymbolTableTest, DefinesAndGetsFunctions) {
    SymbolTable table;
    table.defineFunction("get_answer", create_dummy_func(42.0));
    
    EXPECT_TRUE(table.hasFunction("get_answer"));
    EXPECT_FALSE(table.hasFunction("ghost"));
    
    ICallable* func = table.getFunction("get_answer");
    ASSERT_NE(func, nullptr);
    EXPECT_DOUBLE_EQ((*func)(Vector<Value>()).getNumber(), 42.0);
}

TEST_F(SymbolTableTest, ThrowsOnInvalidFunctionOperations) {
    SymbolTable table;
    
    EXPECT_THROW(table.getFunction("ghost"), std::runtime_error);
    
    table.defineFunction("func", create_dummy_func(1.0));
    EXPECT_THROW(table.defineFunction("func", create_dummy_func(2.0)), std::runtime_error);
    EXPECT_THROW(table.defineFunction("bad", nullptr), std::runtime_error);
}

TEST_F(SymbolTableTest, MoveConstructorTransfersData) {
    SymbolTable original;
    original.defineVariable("var", Value(100.0));
    original.defineFunction("func", create_dummy_func(77.0));

    SymbolTable moved(std::move(original));

    EXPECT_TRUE(moved.hasVariable("var"));
    EXPECT_TRUE(moved.hasFunction("func"));
    EXPECT_DOUBLE_EQ(moved.getVariable("var").getNumber(), 100.0);
}

TEST_F(SymbolTableTest, MoveAssignmentTransfersData) {
    SymbolTable original;
    original.defineVariable("val", Value(55.0));

    SymbolTable destination;
    destination.defineVariable("old", Value(1.0));

    destination = std::move(original);

    EXPECT_TRUE(destination.hasVariable("val"));
    EXPECT_FALSE(destination.hasVariable("old"));
    EXPECT_DOUBLE_EQ(destination.getVariable("val").getNumber(), 55.0);
}

//  ----------------
//      Module
//  ----------------

TEST_F(ModuleTest, StoresNameAndProvidesTableAccess) {
    Module mod("math_plugin");
    EXPECT_EQ(mod.getName(), "math_plugin");
    
    mod.getTable().defineVariable("e", Value(2.71));
    mod.getTable().defineFunction("zero", create_dummy_func(0.0));
    
    const Module& const_mod = mod;
    
    EXPECT_TRUE(const_mod.getTable().hasVariable("e"));
    EXPECT_DOUBLE_EQ(const_mod.getTable().getVariable("e").getNumber(), 2.71);
    EXPECT_TRUE(const_mod.getTable().hasFunction("zero"));
}

//  ------------------------
//      ExecutionContext
//  ------------------------

class ExecutionContextSetupTest : public ExecutionContextTest {
protected:
    Module math_mod{"Math"};
    Module user_mod{"User"};
    
    void SetUp() override {
        math_mod.getTable().defineVariable("pi", Value(3.14), true);
        math_mod.getTable().defineVariable("e", Value(2.71), true);
        math_mod.getTable().defineFunction("math_func", create_dummy_func(1.0));
        
        user_mod.getTable().defineVariable("e", Value(3.0));
        user_mod.getTable().defineFunction("user_func", create_dummy_func(2.0));
    }
};  //  class   ExecutionContextSetupTest

TEST_F(ExecutionContextSetupTest, HandlesLocalMemoryIndependently) {
    ExecutionContext ctx;
    ctx.defineVariable("local_var", Value(100.0));
    ctx.assignVariable("local_var", Value(200.0));
    
    EXPECT_DOUBLE_EQ(ctx.getVariable("local_var").getNumber(), 200.0);
    EXPECT_THROW(ctx.getVariable("unknown"), std::runtime_error);
    EXPECT_THROW(ctx.getFunction("unknown"), std::runtime_error);
}

TEST_F(ExecutionContextSetupTest, SafelyIgnoresNullptrModules) {
    ExecutionContext ctx;
    ctx.importModule(nullptr);
    
    EXPECT_THROW(ctx.getVariable("anything"), std::runtime_error);
}

TEST_F(ExecutionContextSetupTest, ReadsVariablesAndFunctionsFromImports) {
    ExecutionContext ctx;
    ctx.importModule(&math_mod);
    
    EXPECT_DOUBLE_EQ(ctx.getVariable("pi").getNumber(), 3.14);
    EXPECT_DOUBLE_EQ(ctx.getVariable("e").getNumber(), 2.71);
    
    ICallable* f = ctx.getFunction("math_func");
    ASSERT_NE(f, nullptr);
    EXPECT_DOUBLE_EQ((*f)(Vector<Value>()).getNumber(), 1.0);
}

TEST_F(ExecutionContextSetupTest, CannotAssignToImportedVariables) {
    ExecutionContext ctx;
    ctx.importModule(&math_mod);
    
    EXPECT_THROW(ctx.assignVariable("pi", Value(3.0)), std::runtime_error);
}

TEST_F(ExecutionContextSetupTest, LocalVariablesShadowImportedVariables) {
    ExecutionContext ctx;
    ctx.importModule(&math_mod);
    
    ctx.defineVariable("pi", Value(100.0));
    
    EXPECT_DOUBLE_EQ(ctx.getVariable("pi").getNumber(), 100.0);
}

TEST_F(ExecutionContextSetupTest, LaterImportsShadowEarlierImports) {
    ExecutionContext ctx;
    ctx.importModule(&math_mod);
    ctx.importModule(&user_mod);
    
    EXPECT_DOUBLE_EQ(ctx.getVariable("e").getNumber(), 3.0);
}

TEST_F(ExecutionContextSetupTest, ResolvesFunctionsAcrossScopesAndImports) {
    ExecutionContext ctx;
    ctx.importModule(&math_mod);
    ctx.importModule(&user_mod);
    
    EXPECT_DOUBLE_EQ((*ctx.getFunction("math_func"))(Vector<Value>()).getNumber(), 1.0);
    EXPECT_DOUBLE_EQ((*ctx.getFunction("user_func"))(Vector<Value>()).getNumber(), 2.0);
}
