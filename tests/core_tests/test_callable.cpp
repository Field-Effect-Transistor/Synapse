//  /tests/core_tests/test_callable.cpp
#include <gtest/gtest.h>
#include "Callable.hpp"
#include "Exceptions.hpp"

using namespace Synapse;

// =================
//      MOCKS
// =================

Value c_style_math_func(const Vector<Value>& args) {
    double a = args[0].getNumber();
    double b = args[1].getNumber();
    return Value(a + b);
}

class CallableTest : public ::testing::Test {};

using CallableTypesTest     = CallableTest;
using CallableExecutionTest = CallableTest;
using CallableMemoryTest    = CallableTest;


// =================================
//      CALLABLE TYPES SUPPORT
// =================================

TEST_F(CallableTypesTest, WrapsCStyleFunction) {
    CallablePtr func = make_callable(2, &c_style_math_func);
    
    EXPECT_TRUE(static_cast<bool>(*func));
    EXPECT_EQ(func->arity(), 2);

    Vector<Value> args;
    args.push_back(Value(10.0));
    args.push_back(Value(20.0));

    Value res = (*func)(args);
    EXPECT_TRUE(res.isNumber());
    EXPECT_DOUBLE_EQ(res.getNumber(), 30.0);
}

TEST_F(CallableTypesTest, WrapsStatelessLambda) {
    CallablePtr func = make_callable(1, [](const Vector<Value>& args) {
        double val = args[0].getNumber();
        return Value(val * val);
    });

    EXPECT_EQ(func->arity(), 1);

    Vector<Value> args;
    args.push_back(Value(5.0));

    Value res = (*func)(args);
    EXPECT_DOUBLE_EQ(res.getNumber(), 25.0);
}

TEST_F(CallableTypesTest, WrapsStatefulLambda) {
    double state = 100.0;
    
    CallablePtr func = make_callable(1, [&state](const Vector<Value>& args) {
        state += args[0].getNumber(); 
        return Value(state);
    });

    Vector<Value> args;
    args.push_back(Value(50.0));

    Value res = (*func)(args);
    
    EXPECT_DOUBLE_EQ(res.getNumber(), 150.0);
    EXPECT_DOUBLE_EQ(state, 150.0); // Перевіряємо, що стан зовні змінився
}


// =========================
//      LOGIC n ERRORS
// =========================

TEST_F(CallableExecutionTest, VarityAllowsAnyNumberOfArguments) {
    CallablePtr func = make_callable(ICallable::VARITY, [](const Vector<Value>& args) {
        double sum = 0;
        for (const auto& arg : args) sum += arg.getNumber();
        return Value(sum);
    });

    EXPECT_EQ(func->arity(), ICallable::VARITY);

    Vector<Value> args0;
    EXPECT_DOUBLE_EQ((*func)(args0).getNumber(), 0.0);

    Vector<Value> args3;
    args3.push_back(Value(10.0));
    args3.push_back(Value(20.0));
    args3.push_back(Value(30.0));
    EXPECT_DOUBLE_EQ((*func)(args3).getNumber(), 60.0);
}

TEST_F(CallableExecutionTest, ArityMismatchThrowsException) {
    CallablePtr func = make_callable(2, [](const Vector<Value>& args) {
        return Value(0.0);
    });

    Vector<Value> bad_args;
    bad_args.push_back(Value(1.0)); // Передаємо тільки 1 аргумент замість 2

    EXPECT_THROW((*func)(bad_args), ArityMismatchError);
}


// =====================
//      MEMORY MAN
// =====================

TEST_F(CallableMemoryTest, DestroysObjectSafely) {
    bool destroyed = false;
    
    struct TestCallable : public ICallable {
        bool* _flag;
        TestCallable(bool* flag) : _flag(flag) {}
        
        Value operator()(const Vector<Value>&) override { return Value(); }
        size_t arity() const override { return 0; }
        
        explicit operator bool() const override { return true; }
        
        void destroy() override {
            *_flag = true;
            delete this;
        }
    };

    {
        CallablePtr ptr(new TestCallable(&destroyed));
        EXPECT_FALSE(destroyed);
    } 

    EXPECT_TRUE(destroyed);
}
