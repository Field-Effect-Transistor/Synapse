// /tests/core_tests/test_context.cpp
#include <gtest/gtest.h>
#include "Context.hpp"
#include "Callable.hpp"
#include <stdexcept>

using namespace Synapse;

class ContextTest : public ::testing::Test {
protected:
    Vector<Value> empty_args() {
        return Vector<Value>();
    }
    
    Vector<Value> single_arg(double v) {
        Vector<Value> args;
        args.push_back(Value(v));
        return args;
    }
};

//  ================
//      SYMBOLS
//  ================

TEST_F(ContextTest, DefinesAndGetsVariable) {
    Context ctx;
    ctx.defineVariable("x", Value(42.5));
    
    Value v = ctx.getVariable("x");
    EXPECT_TRUE(v.isNumber());
    EXPECT_DOUBLE_EQ(v.getNumber(), 42.5);
}

TEST_F(ContextTest, ThrowsOnGettingUnknownVariable) {
    Context ctx;
    EXPECT_THROW(ctx.getVariable("unknown"), std::runtime_error);
}

TEST_F(ContextTest, AssignsNewValueToExistingVariable) {
    Context ctx;
    ctx.defineVariable("x", Value(10.0));
    ctx.assignVariable("x", Value(99.0));
    
    EXPECT_DOUBLE_EQ(ctx.getVariable("x").getNumber(), 99.0);
}

TEST_F(ContextTest, ThrowsOnAssigningToUnknownVariable) {
    Context ctx;
    EXPECT_THROW(ctx.assignVariable("x", Value(10.0)), std::runtime_error);
}

TEST_F(ContextTest, ThrowsOnRedefiningVariable) {
    Context ctx;
    ctx.defineVariable("x", Value(10.0));
    EXPECT_THROW(ctx.defineVariable("x", Value(20.0)), std::runtime_error);
}

//  ================
//      CONSTANTS
//  ================

TEST_F(ContextTest, ThrowsOnAssigningToConstant) {
    Context ctx;
    ctx.defineVariable("pi", Value(3.14), true);
    
    EXPECT_DOUBLE_EQ(ctx.getVariable("pi").getNumber(), 3.14);
    EXPECT_THROW(ctx.assignVariable("pi", Value(3.0)), std::runtime_error);
}

//  ================
//      FUNCTIONS
//  ================

TEST_F(ContextTest, DefinesAndGetsFunction) {
    Context ctx;
    
    auto my_func = make_callable(1, [](const Vector<Value>& args) {
        return Value(args[0].getNumber() * 2.0);
    });

    ctx.defineFunction("double_it", std::move(my_func));

    ICallable* retrieved = ctx.getFunction("double_it");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->arity(), 1);

    Value res = (*retrieved)(single_arg(10.0));
    EXPECT_DOUBLE_EQ(res.getNumber(), 20.0);
}

TEST_F(ContextTest, ThrowsOnDefiningNullFunction) {
    Context ctx;
    CallablePtr null_func(nullptr);
    EXPECT_THROW(ctx.defineFunction("bad_func", std::move(null_func)), std::runtime_error);
}

TEST_F(ContextTest, ThrowsOnRedefiningFunction) {
    Context ctx;
    auto f1 = make_callable(0, [](const Vector<Value>&) { return Value(1.0); });
    auto f2 = make_callable(0, [](const Vector<Value>&) { return Value(2.0); });

    ctx.defineFunction("func", std::move(f1));
    EXPECT_THROW(ctx.defineFunction("func", std::move(f2)), std::runtime_error);
}

TEST_F(ContextTest, ThrowsOnGettingUnknownFunction) {
    Context ctx;
    EXPECT_THROW(ctx.getFunction("unknown_func"), std::runtime_error);
}

//  ====================
//      SCOPE CHAIN
//  ====================

TEST_F(ContextTest, ChildGetsVariableFromParent) {
    Context parent;
    parent.defineVariable("global_var", Value(100.0));

    Context child(&parent);
    EXPECT_DOUBLE_EQ(child.getVariable("global_var").getNumber(), 100.0);
}

TEST_F(ContextTest, ChildShadowsParentVariable) {
    Context parent;
    parent.defineVariable("x", Value(10.0));

    Context child(&parent);
    child.defineVariable("x", Value(99.0));

    EXPECT_DOUBLE_EQ(child.getVariable("x").getNumber(), 99.0);
    EXPECT_DOUBLE_EQ(parent.getVariable("x").getNumber(), 10.0);
}

TEST_F(ContextTest, AssignVariableStrictScopingRule) {
    Context parent;
    parent.defineVariable("x", Value(10.0));

    Context child(&parent);
    
    EXPECT_THROW(child.assignVariable("x", Value(20.0)), std::runtime_error);
}

TEST_F(ContextTest, ChildGetsFunctionFromParent) {
    Context parent;
    auto my_func = make_callable(0, [](const Vector<Value>&) { return Value(77.0); });
    parent.defineFunction("global_func", std::move(my_func));

    Context child(&parent);
    ICallable* func = child.getFunction("global_func");
    
    ASSERT_NE(func, nullptr);
    EXPECT_DOUBLE_EQ((*func)(empty_args()).getNumber(), 77.0);
}

//  ========================
//      MOVE SEMANTICS
//  ========================

TEST_F(ContextTest, MoveConstructorTransfersData) {
    Context original;
    original.defineVariable("x", Value(55.5));

    Context moved(std::move(original));

    EXPECT_DOUBLE_EQ(moved.getVariable("x").getNumber(), 55.5);
}
