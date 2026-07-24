#include <gtest/gtest.h>
#include "Value.hpp"

using namespace Hermes;

// --- MOCK КЛАС ДЛЯ ТЕСТУВАННЯ ПАМ'ЯТІ ---
// Він рахує, скільки разів його створили, клонували та знищили.
struct MockObject : public ICustomValue {
    static int active_instances;
    static int clone_calls;
    static int destroy_calls;
    
    int dummy_data;

    MockObject(int d) : dummy_data(d) { active_instances++; }
    ~MockObject() override { active_instances--; }

    ICustomValue* clone() override {
        clone_calls++;
        return new MockObject(dummy_data);
    }

    void destroy() override {
        destroy_calls++;
        delete this;
    }

    static void resetCounters() {
        active_instances = 0;
        clone_calls = 0;
        destroy_calls = 0;
    }
    
    const char* to_str() const override { return "Mock"; }
};

int MockObject::active_instances = 0;
int MockObject::clone_calls = 0;
int MockObject::destroy_calls = 0;


// --- ТЕСТИ ---

class ValueTest : public ::testing::Test {
protected:
    void SetUp() override {
        MockObject::resetCounters();
    }
};

TEST_F(ValueTest, DefaultConstructor) {
    Value v;
    EXPECT_TRUE(v.isNumber());
    EXPECT_DOUBLE_EQ(v.getNumber(), 0.0);
}

TEST_F(ValueTest, PrimitiveConstructors) {
    Value v1(42.5);
    EXPECT_TRUE(v1.isNumber());
    EXPECT_DOUBLE_EQ(v1.getNumber(), 42.5);

    Value v2(true);
    EXPECT_TRUE(v2.isBoolean());
    EXPECT_TRUE(v2.getBool());
}

TEST_F(ValueTest, ExceptionOnWrongTypeAccess) {
    Value v(10.0);
    EXPECT_THROW(v.getBool(), std::runtime_error);
    EXPECT_THROW(v.getCustom(), std::runtime_error);

    Value vb(false);
    EXPECT_THROW(vb.getNumber(), std::runtime_error);
}

TEST_F(ValueTest, CustomObjectLifecycle) {
    {
        Value v(new MockObject(99));
        EXPECT_TRUE(v.isCustom());
        EXPECT_EQ(MockObject::active_instances, 1);
        
        MockObject* obj = static_cast<MockObject*>(v.getCustom());
        EXPECT_EQ(obj->dummy_data, 99);
    }

    EXPECT_EQ(MockObject::active_instances, 0);
    EXPECT_EQ(MockObject::destroy_calls, 1);
}

TEST_F(ValueTest, CopyConstructor) {
    Value original(new MockObject(1));
    EXPECT_EQ(MockObject::active_instances, 1);

    Value copy(original);

    // obj via clone()
    EXPECT_TRUE(copy.isCustom());
    EXPECT_EQ(MockObject::clone_calls, 1);
    EXPECT_EQ(MockObject::active_instances, 2);
    
    EXPECT_NE(original.getCustom(), copy.getCustom());
    EXPECT_EQ(static_cast<MockObject*>(copy.getCustom())->dummy_data, 1);
}

TEST_F(ValueTest, CopyAssignment) {
    Value v1(new MockObject(10));
    Value v2(new MockObject(20));
    
    EXPECT_EQ(MockObject::active_instances, 2);

    v1 = v2;

    EXPECT_EQ(MockObject::destroy_calls, 1);
    EXPECT_EQ(MockObject::clone_calls, 1);
    EXPECT_EQ(MockObject::active_instances, 2);
}

TEST_F(ValueTest, MoveConstructor) {
    Value original(new MockObject(42));
    
    Value moved(std::move(original));

    EXPECT_EQ(MockObject::clone_calls, 0);
    EXPECT_EQ(MockObject::active_instances, 1);

    EXPECT_TRUE(moved.isCustom());
    EXPECT_EQ(static_cast<MockObject*>(moved.getCustom())->dummy_data, 42);

    EXPECT_TRUE(original.isNumber());
    EXPECT_DOUBLE_EQ(original.getNumber(), 0.0);
}

TEST_F(ValueTest, MoveAssignment) {
    Value v1(new MockObject(100));
    Value v2(new MockObject(200));

    v1 = std::move(v2);

    EXPECT_EQ(MockObject::destroy_calls, 1);
    EXPECT_EQ(MockObject::clone_calls, 0);
    EXPECT_EQ(MockObject::active_instances, 1);

    EXPECT_TRUE(v1.isCustom());
    EXPECT_TRUE(v2.isNumber());
}

TEST_F(ValueTest, SelfAssignment) {
    Value v_copy(42.0);
    v_copy = v_copy; 
    EXPECT_DOUBLE_EQ(v_copy.getNumber(), 42.0);

    Value v_move(new MockObject(5));
    v_move = std::move(v_move); 
    EXPECT_TRUE(v_move.isCustom());
}

TEST_F(ValueTest, CopyPrimitives) {
    Value v_num(10.5);
    Value v_num_copy(v_num); 
    EXPECT_DOUBLE_EQ(v_num_copy.getNumber(), 10.5);

    Value v_bool(true);
    Value v_bool_assign;
    v_bool_assign = v_bool; 
    EXPECT_TRUE(v_bool_assign.getBool());
}

TEST_F(ValueTest, AssignToPrimitiveDestination) {
    Value dest_copy(1.0);
    Value src_copy(new MockObject(77));
    dest_copy = src_copy; 
    EXPECT_TRUE(dest_copy.isCustom());

    Value dest_move(2.0);
    Value src_move(new MockObject(88));
    dest_move = std::move(src_move);
    EXPECT_TRUE(dest_move.isCustom());
}

TEST_F(ValueTest, GetTypeEnum) {
    Value v(3.14);
    EXPECT_EQ(v.type(), Value::Type::NUMBER);
    
    Value vb(false);
    EXPECT_EQ(vb.type(), Value::Type::BOOLEAN);
}
