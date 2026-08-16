// /tests/core_tests/test_context.cpp
#include <gtest/gtest.h>
#include "synapse/ContextManager.hpp"
#include "synapse/Context.hpp"
#include "synapse/Callable.hpp"
#include <stdexcept>

using namespace Synapse;
using CallablePtr = ICallable::Ptr;

namespace Synapse {
    class ContextTestWrapper final {
    private:
        Context _context;

    public:
        explicit ContextTestWrapper(ContextTestWrapper* parent = nullptr) 
            : _context(parent ? &(parent->_context) : nullptr) {}
            
        ~ContextTestWrapper() = default;

        ContextTestWrapper(const ContextTestWrapper&) = delete;
        ContextTestWrapper& operator=(const ContextTestWrapper&) = delete;

        ContextTestWrapper(ContextTestWrapper&&) noexcept = default;
        ContextTestWrapper& operator=(ContextTestWrapper&&) noexcept = default;

        void defineVariable(const char* name, Value val, bool is_const = false) {
            _context.defineVariable(name, std::move(val), is_const);
        }

        void assignVariable(const char* name, Value val) {
            _context.assignVariable(name, std::move(val));
        }

        Value getVariable(const char* name) const {
            return _context.getVariable(name);
        }

        void defineFunction(const char* name, CallablePtr function) {
            _context.defineFunction(name, std::move(function));
        }

        ICallable* getFunction(const char* name) const {
            return _context.getFunction(name);
        }

        Context* get() {
            return &_context;
        }
    };
}

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

using ContextSymbolsTest        = ContextTest;
using ContextConstantsTest      = ContextTest;
using ContextFunctionsTest      = ContextTest;
using ContextScopeChainTest     = ContextTest;
using ContextMoveSemanticsTest  = ContextTest;


//  ================
//      SYMBOLS
//  ================

TEST_F(ContextSymbolsTest, DefinesAndGetsVariable) {
    ContextTestWrapper ctx;
    ctx.defineVariable("x", Value(42.5));
    
    Value v = ctx.getVariable("x");
    EXPECT_TRUE(v.isNumber());
    EXPECT_DOUBLE_EQ(v.getNumber(), 42.5);
}

TEST_F(ContextSymbolsTest, ThrowsOnGettingUnknownVariable) {
    ContextTestWrapper ctx;
    EXPECT_THROW(ctx.getVariable("unknown"), std::runtime_error);
}

TEST_F(ContextSymbolsTest, AssignsNewValueToExistingVariable) {
    ContextTestWrapper ctx;
    ctx.defineVariable("x", Value(10.0));
    ctx.assignVariable("x", Value(99.0));
    
    EXPECT_DOUBLE_EQ(ctx.getVariable("x").getNumber(), 99.0);
}

TEST_F(ContextSymbolsTest, ThrowsOnAssigningToUnknownVariable) {
    ContextTestWrapper ctx;
    EXPECT_THROW(ctx.assignVariable("x", Value(10.0)), std::runtime_error);
}

TEST_F(ContextSymbolsTest, ThrowsOnRedefiningVariable) {
    ContextTestWrapper ctx;
    ctx.defineVariable("x", Value(10.0));
    EXPECT_THROW(ctx.defineVariable("x", Value(20.0)), std::runtime_error);
}

//  ================
//      CONSTANTS
//  ================

TEST_F(ContextConstantsTest, ThrowsOnAssigningToConstant) {
    ContextTestWrapper ctx;
    ctx.defineVariable("pi", Value(3.14), true);
    
    EXPECT_DOUBLE_EQ(ctx.getVariable("pi").getNumber(), 3.14);
    EXPECT_THROW(ctx.assignVariable("pi", Value(3.0)), std::runtime_error);
}

//  ================
//      FUNCTIONS
//  ================

TEST_F(ContextFunctionsTest, DefinesAndGetsFunction) {
    ContextTestWrapper ctx;
    
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

TEST_F(ContextFunctionsTest, ThrowsOnDefiningNullFunction) {
    ContextTestWrapper ctx;
    CallablePtr null_func(nullptr);
    EXPECT_THROW(ctx.defineFunction("bad_func", std::move(null_func)), std::runtime_error);
}

TEST_F(ContextFunctionsTest, ThrowsOnRedefiningFunction) {
    ContextTestWrapper ctx;
    auto f1 = make_callable(0, [](const Vector<Value>&) { return Value(1.0); });
    auto f2 = make_callable(0, [](const Vector<Value>&) { return Value(2.0); });

    ctx.defineFunction("func", std::move(f1));
    EXPECT_THROW(ctx.defineFunction("func", std::move(f2)), std::runtime_error);
}

TEST_F(ContextFunctionsTest, ThrowsOnGettingUnknownFunction) {
    ContextTestWrapper ctx;
    EXPECT_THROW(ctx.getFunction("unknown_func"), std::runtime_error);
}

//  ====================
//      SCOPE CHAIN
//  ====================

TEST_F(ContextScopeChainTest, ChildGetsVariableFromParent) {
    ContextTestWrapper parent;
    parent.defineVariable("global_var", Value(100.0));

    ContextTestWrapper child(&parent);
    EXPECT_DOUBLE_EQ(child.getVariable("global_var").getNumber(), 100.0);
}

TEST_F(ContextScopeChainTest, ChildShadowsParentVariable) {
    ContextTestWrapper parent;
    parent.defineVariable("x", Value(10.0));

    ContextTestWrapper child(&parent);
    child.defineVariable("x", Value(99.0));

    EXPECT_DOUBLE_EQ(child.getVariable("x").getNumber(), 99.0);
    EXPECT_DOUBLE_EQ(parent.getVariable("x").getNumber(), 10.0);
}

TEST_F(ContextScopeChainTest, AssignVariableStrictScopingRule) {
    ContextTestWrapper parent;
    parent.defineVariable("x", Value(10.0));

    ContextTestWrapper child(&parent);
    
    EXPECT_THROW(child.assignVariable("x", Value(20.0)), std::runtime_error);
}

TEST_F(ContextScopeChainTest, ChildGetsFunctionFromParent) {
    ContextTestWrapper parent;
    auto my_func = make_callable(0, [](const Vector<Value>&) { return Value(77.0); });
    parent.defineFunction("global_func", std::move(my_func));

    ContextTestWrapper child(&parent);
    ICallable* func = child.getFunction("global_func");
    
    ASSERT_NE(func, nullptr);
    EXPECT_DOUBLE_EQ((*func)(empty_args()).getNumber(), 77.0);
}

//  ========================
//      MOVE SEMANTICS
//  ========================

TEST_F(ContextMoveSemanticsTest, MoveConstructorTransfersData) {
    ContextTestWrapper original;
    original.defineVariable("x", Value(55.5));

    ContextTestWrapper moved(std::move(original));

    EXPECT_DOUBLE_EQ(moved.getVariable("x").getNumber(), 55.5);
}

TEST_F(ContextMoveSemanticsTest, MoveAssignmentTransfersData) {
    ContextTestWrapper ctx1;
    ctx1.defineVariable("magic", Value(77.7));

    ContextTestWrapper ctx2;
    ctx2 = std::move(ctx1);

    EXPECT_DOUBLE_EQ(ctx2.getVariable("magic").getNumber(), 77.7);
}
