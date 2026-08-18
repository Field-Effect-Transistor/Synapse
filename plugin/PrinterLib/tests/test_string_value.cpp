// /plugin/PrinterLib/tests/test_string_value.cpp
#include <gtest/gtest.h>
#include "StringValue.hpp"

using namespace PrinterLib;
using namespace Synapse;

class StringValueTest : public ::testing::Test {
protected:
    // Хелпер для безпечного знищення кастомного об'єкта, 
    // як це робиться всередині Hermes::Value
    void safeDestroy(ICustomValue* ptr) {
        if (ptr) ptr->destroy();
    }
};

// ==========================================
// Тестування конструкторів та базових методів
// ==========================================

TEST_F(StringValueTest, DefaultConstructorCreatesEmptyString) {
    StringValue* sv = new StringValue();
    
    EXPECT_STREQ(sv->to_str(), "");
    
    safeDestroy(sv);
}

TEST_F(StringValueTest, StringConstructorStoresData) {
    std::string test_str = "Hello, Synapse!";
    StringValue* sv = new StringValue(test_str);
    
    EXPECT_STREQ(sv->to_str(), "Hello, Synapse!");
    
    safeDestroy(sv);
}

TEST_F(StringValueTest, StringConstructorWithMoveSemantics) {
    std::string test_str = "Moved String";
    const char* original_data_ptr = test_str.data();
    
    // Переміщуємо рядок
    StringValue* sv = new StringValue(std::move(test_str));
    
    // std::string після move може бути порожнім
    EXPECT_TRUE(test_str.empty()); 
    EXPECT_STREQ(sv->to_str(), "Moved String");
    
    safeDestroy(sv);
}

// ==========================================
// Тестування копіювання та клонування
// ==========================================

TEST_F(StringValueTest, CopyConstructorCreatesIndependentCopy) {
    StringValue original("Original Data");
    StringValue copy(original);
    
    EXPECT_STREQ(copy.to_str(), "Original Data");
    
    // Перевіряємо, що це різні об'єкти в пам'яті
    EXPECT_NE(original.to_str(), copy.to_str()); 
}

TEST_F(StringValueTest, CloneMethodReturnsIndependentCopy) {
    StringValue* original = new StringValue("Data for cloning");
    
    ICustomValue* cloned = original->clone();
    
    ASSERT_NE(cloned, nullptr);
    EXPECT_STREQ(cloned->to_str(), "Data for cloning");
    
    // Вказівники на самі об'єкти мають бути різними
    EXPECT_NE(original, cloned);
    
    safeDestroy(original);
    safeDestroy(cloned);
}

// ==========================================
// Тестування інтеграції з Synapse::Value (Опціонально, але корисно)
// ==========================================

#include "synapse/Value.hpp"

TEST_F(StringValueTest, IntegratesSafelyWithSynapseValue) {
    // 1. Створюємо Value, що володіє StringValue
    Value val(new StringValue("Test inside Value"));
    
    EXPECT_TRUE(val.isCustom());
    
    // 2. Отримуємо кастомний об'єкт
    ICustomValue* custom_ptr = val.getCustom();
    ASSERT_NE(custom_ptr, nullptr);
    
    // 3. Перевіряємо дані
    EXPECT_STREQ(custom_ptr->to_str(), "Test inside Value");
    EXPECT_EQ(val.to_str(), "Test inside Value");
    
    // Коли 'val' вийде з області видимості, він автоматично викличе
    // custom_ptr->destroy(). Якщо ASan мовчить - витоків немає!
}
