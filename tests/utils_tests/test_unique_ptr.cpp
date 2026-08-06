#include <gtest/gtest.h>
#include "UniquePtr.hpp"
#include <utility>

using namespace Synapse;

struct LifetimeTracker {
    bool* is_destroyed;
    int value;

    LifetimeTracker(bool* flag, int v = 0) : is_destroyed(flag), value(v) {
        if (is_destroyed) *is_destroyed = false;
    }
    ~LifetimeTracker() {
        if (is_destroyed) *is_destroyed = true;
    }
    
    int getValue() const { return value; }
};

TEST(UniquePtrTest, DefaultConstructionAndBool) {
    UniquePtr<int> ptr;
    UniquePtr<int> ptr_null(nullptr);

    EXPECT_FALSE(ptr);
    EXPECT_FALSE(ptr_null);
    EXPECT_EQ(ptr.get(), nullptr);
}

TEST(UniquePtrTest, RawPointerConstructionAndDestruction) {
    bool destroyed = false;
    {
        UniquePtr<LifetimeTracker> ptr(new LifetimeTracker(&destroyed));
        EXPECT_TRUE(ptr);
        EXPECT_NE(ptr.get(), nullptr);
        EXPECT_FALSE(destroyed);
    }

    EXPECT_TRUE(destroyed);
}

TEST(UniquePtrTest, PointerAccessOperators) {
    UniquePtr<LifetimeTracker> ptr(new LifetimeTracker(nullptr, 42));

    // Перевірка operator->
    EXPECT_EQ(ptr->getValue(), 42);

    // Перевірка operator*
    EXPECT_EQ((*ptr).value, 42);
}

TEST(UniquePtrTest, MoveConstructor) {
    bool destroyed = false;
    UniquePtr<LifetimeTracker> ptr1(new LifetimeTracker(&destroyed, 100));
    
    // Переміщуємо ptr1 у ptr2
    UniquePtr<LifetimeTracker> ptr2(std::move(ptr1));

    // ptr1 має стати порожнім
    EXPECT_FALSE(ptr1);
    EXPECT_EQ(ptr1.get(), nullptr);

    // ptr2 має перейняти володіння
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2->getValue(), 100);
    EXPECT_FALSE(destroyed);
}

TEST(UniquePtrTest, MoveAssignment) {
    bool destr1 = false;
    bool destr2 = false;

    UniquePtr<LifetimeTracker> ptr1(new LifetimeTracker(&destr1, 11));
    UniquePtr<LifetimeTracker> ptr2(new LifetimeTracker(&destr2, 22));

    // Переміщуємо ptr2 у ptr1
    ptr1 = std::move(ptr2);

    // Старий об'єкт з ptr1 мав бути ЗНИЩЕНИЙ
    EXPECT_TRUE(destr1);
    
    // ptr2 має стати порожнім
    EXPECT_FALSE(ptr2);
    
    // ptr1 тепер володіє об'єктом, який був у ptr2
    EXPECT_TRUE(ptr1);
    EXPECT_EQ(ptr1->getValue(), 22);
    EXPECT_FALSE(destr2);
}

TEST(UniquePtrTest, ReleaseMethod) {
    bool destroyed = false;
    LifetimeTracker* raw_ptr = nullptr;

    {
        UniquePtr<LifetimeTracker> ptr(new LifetimeTracker(&destroyed, 99));
        raw_ptr = ptr.release();

        EXPECT_FALSE(ptr);
        EXPECT_EQ(ptr.get(), nullptr);
    }

    EXPECT_FALSE(destroyed);
    EXPECT_EQ(raw_ptr->getValue(), 99);

    delete raw_ptr;
}

TEST(UniquePtrTest, ResetMethod) {
    bool destr1 = false;
    bool destr2 = false;

    UniquePtr<LifetimeTracker> ptr(new LifetimeTracker(&destr1, 1));
    
    // Скидаємо на новий вказівник
    ptr.reset(new LifetimeTracker(&destr2, 2));

    // Старий об'єкт має бути знищений
    EXPECT_TRUE(destr1);
    
    // Новий об'єкт живий
    EXPECT_FALSE(destr2);
    EXPECT_EQ(ptr->getValue(), 2);

    // Скидаємо в nullptr
    ptr.reset(nullptr);
    EXPECT_TRUE(destr2);
    EXPECT_FALSE(ptr);
}

struct CustomMockDeleter {
    static int delete_calls;
    void operator()(int* p) const {
        delete_calls++;
        delete p;
    }
};
int CustomMockDeleter::delete_calls = 0;

TEST(UniquePtrTest, CustomDeleterSupport) {
    CustomMockDeleter::delete_calls = 0;
    {
        UniquePtr<int, CustomMockDeleter> ptr(new int(5));
    }

    EXPECT_EQ(CustomMockDeleter::delete_calls, 1);
}
