//  /tests/utils_tests/test_vector.cpp
#include <gtest/gtest.h>
#include "Vector.hpp"

#include <cstdint>

using namespace Synapse;

//  ============
//      Mock
//  ============

struct LifeTimeSpy {
    inline static size_t created_count = 0;
    inline static size_t copied_count = 0;
    inline static size_t moved_count = 0;
    inline static size_t destroyed_count = 0;

    inline static uint64_t global_id_counter = 0;
    inline uint64_t gen_id() { return ++global_id_counter; }
    inline static void reset() {
        created_count = 0;
        copied_count = 0;
        moved_count = 0;
        destroyed_count = 0;
    }

    uint64_t id;

    LifeTimeSpy() {
        id = gen_id();
        ++created_count;
    }

    LifeTimeSpy(const LifeTimeSpy& spy) {
        id = spy.id;
        ++copied_count;
    }

    LifeTimeSpy(LifeTimeSpy&& spy) noexcept {
        id = spy.id;
        spy.id = 0;
        ++moved_count;
    }

    ~LifeTimeSpy() {
        id = 0;
        ++destroyed_count;
    }

    bool operator==(const LifeTimeSpy& other) const {
        return id == other.id;
    }
};

struct KamikazeObject {
    inline static size_t throw_on_copy = 0;
    inline static size_t copied_counter = 0;
    inline static uint64_t global_id_counter = 0;

    static void set_throw_on_copy(size_t i) { throw_on_copy = i; }
    static void reset() { 
        throw_on_copy = 0; 
        copied_counter = 0; 
        global_id_counter = 0; 
    }

    inline uint64_t gen_id() { return ++global_id_counter; }

    uint64_t id = 0;

    KamikazeObject() { id = gen_id(); }

    KamikazeObject(const KamikazeObject& obj) {
        id = obj.id;
        ++copied_counter;
        if (throw_on_copy > 0 && copied_counter == throw_on_copy) {
            throw std::runtime_error("KamikazeObj with id = " + std::to_string(id));
        }
    }

    KamikazeObject(KamikazeObject&& obj) noexcept {
        id = obj.id;
        obj.id = 0;
    }

    ~KamikazeObject() { id = 0; }
};


struct UnsafeKamikaze {
    static inline int copy_count = 0;
    static inline int throw_limit = 0;

    UnsafeKamikaze() = default;
    
    UnsafeKamikaze(const UnsafeKamikaze&) {
        copy_count++;
        if (throw_limit > 0 && copy_count == throw_limit) {
            throw std::runtime_error("Boom during reserve!");
        }
    }
};


class VectorTest : public testing::Test {
protected:
    void SetUp() override {
        LifeTimeSpy::reset();
        KamikazeObject::reset();
    }
};

using VectorCapacityTest  = VectorTest;
using VectorAccessTest    = VectorTest;
using VectorModifiersTest = VectorTest;
using VectorMemoryTest    = VectorTest;

//  ========================
//      Capacity group
//  ========================

TEST_F(VectorCapacityTest, DefaultConstructorInitializesEmpty) {
    Vector<int> v;

    EXPECT_EQ(v.size(), 0);
    EXPECT_EQ(v.capacity(), 0);
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.data(), nullptr);
}

TEST_F(VectorCapacityTest, InitializerListConstructsCorrectly) {
    Vector<int> v_init = {1, 2, 3};

    EXPECT_EQ(v_init.size(), 3);
    EXPECT_EQ(v_init.capacity(), 3);
    EXPECT_FALSE(v_init.empty());
    EXPECT_NE(v_init.data(), nullptr);
}


//  ========================
//      Access group
//  ========================

TEST_F(VectorAccessTest, ElementAccessOperatorsWork) {
    Vector<int> v_init = {1, 2, 3};

    EXPECT_EQ(v_init[0], 1);
    EXPECT_EQ(v_init[1], 2);
    EXPECT_EQ(v_init[2], 3);

    EXPECT_EQ(v_init.at(0), 1);
    EXPECT_EQ(v_init.at(1), 2);
    EXPECT_EQ(v_init.at(2), 3);
}

TEST_F(VectorAccessTest, AtThrowsOutOfRangeWhenIndexIsInvalid) {
    Vector<int> v_init = {1, 2, 3};

    EXPECT_THROW(v_init.at(69), std::out_of_range);
}

TEST_F(VectorAccessTest, RangeBasedForLoopWorks) {
    Vector<int> v_init = {1, 2, 3};
    int acc = 0;

    for(const auto& m : v_init) {
        acc += m;
    }

    EXPECT_EQ(acc, 6);
}

TEST_F(VectorAccessTest, ConstAtThrowsOutOfRangeWhenIndexIsInvalid) {
    const Vector<int> v_const = {1, 2, 3};

    EXPECT_EQ(v_const.at(0), 1);
    EXPECT_EQ(v_const.at(2), 3);

    EXPECT_THROW(v_const.at(69), std::out_of_range);
}


//  ========================
//      Modifiers group
//  ========================

TEST_F(VectorModifiersTest, PushBackCopiesAndMovesCorrectly) {
    Vector<LifeTimeSpy> v;
    LifeTimeSpy spy;
    
    v.push_back(spy);
    EXPECT_EQ(LifeTimeSpy::copied_count, 1);
    EXPECT_EQ(LifeTimeSpy::moved_count, 0);

    v.push_back(LifeTimeSpy()); 
    EXPECT_EQ(LifeTimeSpy::moved_count, 2); 

    v.clear();
    EXPECT_EQ(LifeTimeSpy::destroyed_count, 4);
}

TEST_F(VectorModifiersTest, PopBackDestroysLastElement) {
    Vector<LifeTimeSpy> v;
    for (size_t i = 0; i < 3; ++i) {
        v.push_back(LifeTimeSpy());
    }
    auto destroyed_before_pop = LifeTimeSpy::destroyed_count;

    v.pop_back();
    
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v.capacity(), 4);
    EXPECT_EQ(LifeTimeSpy::destroyed_count, destroyed_before_pop + 1);
}

TEST_F(VectorModifiersTest, ClearDestroysAllElementsButKeepsCapacity) {
    Vector<LifeTimeSpy> v;
    for (size_t i = 0; i < 3; ++i) {
        v.push_back(LifeTimeSpy());
    }
    auto destroyed_before_clear = LifeTimeSpy::destroyed_count;
    
    v.clear();
    
    EXPECT_EQ(v.size(), 0);
    EXPECT_EQ(v.capacity(), 4);
    EXPECT_EQ(LifeTimeSpy::destroyed_count, destroyed_before_clear + 3);
}

TEST_F(VectorModifiersTest, ResizeChangesSizeAndInitializesCorrectly) {
    Vector<LifeTimeSpy> v;
    
    for (int i = 0; i < 5; ++i) v.push_back(LifeTimeSpy());
    EXPECT_EQ(v.size(), 5);
    size_t cap_before = v.capacity();

    //  Зменшення
    auto destroyed_before = LifeTimeSpy::destroyed_count;
    v.resize(3);
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v.capacity(), cap_before);
    EXPECT_EQ(LifeTimeSpy::destroyed_count, destroyed_before + 2); // 2 хвости вбито

    //  Збільшення в межах поточного capacity
    auto created_before = LifeTimeSpy::created_count;
    v.resize(5);
    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v.capacity(), cap_before);
    EXPECT_EQ(LifeTimeSpy::created_count, created_before + 2); // Створено 2 нові дефолтні об'єкти

    //  Збільшення з реалокацією
    v.resize(20);
    EXPECT_EQ(v.size(), 20);
    EXPECT_GE(v.capacity(), 20);
}


//  ====================
//      Memory group
//  ====================

TEST_F(VectorMemoryTest, DestructorFreesAllElements) {
    {
        Vector<LifeTimeSpy> v;
        v.reserve(10); 
        
        for (size_t i = 0; i < 10; ++i) {
            v.push_back(LifeTimeSpy());
        }
        
        EXPECT_EQ(LifeTimeSpy::created_count, 10);
        EXPECT_EQ(LifeTimeSpy::moved_count, 10);
    }

    EXPECT_EQ(LifeTimeSpy::destroyed_count, 20);
}

TEST_F(VectorMemoryTest, CopyConstructorCreatesDeepCopy) {
    Vector<LifeTimeSpy> v1;
    v1.reserve(10);
    for (size_t i = 0; i < v1.capacity(); ++i) {
        v1.push_back(LifeTimeSpy());
    }
    auto copied = LifeTimeSpy::copied_count;

    Vector<LifeTimeSpy> v2(v1);

    EXPECT_NE(v1.data(), v2.data());
    for (size_t i = 0; i < v1.size(); ++i) {
        EXPECT_EQ(v1[i], v2[i]);
    }
    EXPECT_EQ(LifeTimeSpy::copied_count, copied + 10);
}

TEST_F(VectorMemoryTest, MoveConstructorTransfersOwnershipWithoutCopies) {
    Vector<LifeTimeSpy> v1;
    v1.push_back(LifeTimeSpy());
    v1.push_back(LifeTimeSpy());
    v1.push_back(LifeTimeSpy());

    auto copied_before = LifeTimeSpy::copied_count;
    auto moved_before = LifeTimeSpy::moved_count;

    Vector<LifeTimeSpy> v2(std::move(v1));

    EXPECT_EQ(v1.size(), 0);
    EXPECT_EQ(v1.capacity(), 0);
    EXPECT_EQ(v1.data(), nullptr);

    EXPECT_EQ(v2.size(), 3);
    EXPECT_NE(v2.data(), nullptr);

    // Самі елементи не повинні були ні копіюватися, ні переміщуватися 
    // (перемістився лише вказівник на сиру пам'ять!)
    EXPECT_EQ(LifeTimeSpy::copied_count, copied_before);
    EXPECT_EQ(LifeTimeSpy::moved_count, moved_before);
}

TEST_F(VectorMemoryTest, CopyAssignmentCreatesDeepCopyAndDestroysOldData) {
    Vector<LifeTimeSpy> v1;
    v1.push_back(LifeTimeSpy());
    v1.push_back(LifeTimeSpy());

    Vector<LifeTimeSpy> v2;
    v2.push_back(LifeTimeSpy());
    
    auto destroyed_before = LifeTimeSpy::destroyed_count;
    auto copied_before = LifeTimeSpy::copied_count;

    v2 = v1;

    EXPECT_EQ(v2.size(), 2);
    EXPECT_NE(v1.data(), v2.data());
    EXPECT_EQ(LifeTimeSpy::destroyed_count, destroyed_before + 1);
    EXPECT_EQ(LifeTimeSpy::copied_count, copied_before + 2);
}

TEST_F(VectorMemoryTest, MoveAssignmentTransfersOwnershipAndDestroysOldData) {
    Vector<LifeTimeSpy> v1;
    v1.push_back(LifeTimeSpy());
    v1.push_back(LifeTimeSpy());

    Vector<LifeTimeSpy> v2;
    v2.push_back(LifeTimeSpy()); 
    
    auto destroyed_before = LifeTimeSpy::destroyed_count;
    auto copied_before = LifeTimeSpy::copied_count;
    auto moved_before = LifeTimeSpy::moved_count;

    v2 = std::move(v1);

    EXPECT_EQ(v1.size(), 0);
    EXPECT_EQ(v1.data(), nullptr);
    EXPECT_EQ(v2.size(), 2);

    EXPECT_EQ(LifeTimeSpy::destroyed_count, destroyed_before + 1); // Старий елемент з v2 вбито
    EXPECT_EQ(LifeTimeSpy::copied_count, copied_before); // Жодних копій
    EXPECT_EQ(LifeTimeSpy::moved_count, moved_before);   // Жодних переміщень самих об'єктів
}

TEST_F(VectorMemoryTest, SelfAssignmentIsSafe) {
    Vector<int> v = {1, 2, 3};
    int* original_data = v.data();

    v = v; // Copy self-assignment
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v.data(), original_data);

    v = std::move(v); // Move self-assignment
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v.data(), original_data);
}

TEST_F(VectorMemoryTest, ShrinkToFitFreesUnusedMemory) {
    //  Вектор ідеально заповнений
    Vector<int> v_perfect = {1, 2, 3};
    v_perfect.shrink_to_fit();
    EXPECT_EQ(v_perfect.capacity(), 3);

    //  Вектор порожній, але пам'ять виділена
    Vector<int> v_empty;
    v_empty.reserve(100);
    v_empty.shrink_to_fit();
    EXPECT_EQ(v_empty.capacity(), 0);
    EXPECT_EQ(v_empty.data(), nullptr);

    //  Нормальне стиснення з об'єктами
    Vector<LifeTimeSpy> v_normal;
    v_normal.reserve(100);
    for (int i = 0; i < 5; ++i) v_normal.push_back(LifeTimeSpy());
    
    auto moved_before = LifeTimeSpy::moved_count;
    auto destroyed_before = LifeTimeSpy::destroyed_count;

    v_normal.shrink_to_fit();

    EXPECT_EQ(v_normal.size(), 5);
    EXPECT_EQ(v_normal.capacity(), 5); // Стислося ідеально!
    EXPECT_EQ(LifeTimeSpy::moved_count, moved_before + 5); // Об'єкти переїхали
    EXPECT_EQ(LifeTimeSpy::destroyed_count, destroyed_before + 5); // Старі об'єкти вбито
}

TEST_F(VectorMemoryTest, ShrinkToFitProvidesStrongExceptionGuarantee) {
    Vector<UnsafeKamikaze> v;
    v.reserve(10);
    v.push_back(UnsafeKamikaze());
    v.push_back(UnsafeKamikaze());
    v.push_back(UnsafeKamikaze());

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v.capacity(), 10);
    UnsafeKamikaze* old_data_ptr = v.data();

    UnsafeKamikaze::copy_count = 0;
    UnsafeKamikaze::throw_limit = 2;

    bool exception_caught = false;
    try {
        v.shrink_to_fit();
    } catch (const std::runtime_error&) {
        exception_caught = true;
    }

    EXPECT_TRUE(exception_caught);

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v.capacity(), 10);
    EXPECT_EQ(v.data(), old_data_ptr); // Залишився у старому буфері
}


//  ================================
//      Reallocation & Exceptions
//  ================================

TEST_F(VectorMemoryTest, PushBackAutomaticallyGrowsCapacity) {
    Vector<int> v;
    EXPECT_EQ(v.capacity(), 0);

    v.push_back(1);
    EXPECT_EQ(v.capacity(), 1);

    v.push_back(2);
    EXPECT_EQ(v.capacity(), 2);

    v.push_back(3);
    EXPECT_EQ(v.capacity(), 4);

    v.push_back(4);
    EXPECT_EQ(v.capacity(), 4);

    v.push_back(5);
    EXPECT_EQ(v.capacity(), 8);
}

TEST_F(VectorMemoryTest, ReallocationUsesMoveSemanticsWhenNoexcept) {
    Vector<LifeTimeSpy> v;
    auto copied_before = LifeTimeSpy::copied_count;
    
    for (int i = 0; i < 5; ++i) {
        v.push_back(LifeTimeSpy());
    }

    // Під час реалокацій не має бути жодного копіювання, тільки переміщення!
    EXPECT_EQ(LifeTimeSpy::copied_count, copied_before);
}

TEST_F(VectorMemoryTest, CopyConstructorProvidesStrongExceptionGuarantee) {
    Vector<KamikazeObject> v1;
    for (int i = 0; i < 5; ++i) {
        v1.push_back(KamikazeObject());
    }

    KamikazeObject::set_throw_on_copy(3);

    bool exception_caught = false;
    try {
        Vector<KamikazeObject> v2(v1);
    } catch (const std::runtime_error& e) {
        exception_caught = true;
    }

    EXPECT_TRUE(exception_caught);
    
    // Оригінальний вектор має залишитися повністю цілим
    EXPECT_EQ(v1.size(), 5);
    EXPECT_NE(v1.data(), nullptr);
}

TEST_F(VectorMemoryTest, ReserveProvidesStrongExceptionGuarantee) {
    Vector<UnsafeKamikaze> v;
    v.reserve(2);
    v.push_back(UnsafeKamikaze());
    v.push_back(UnsafeKamikaze());

    UnsafeKamikaze::copy_count = 0;
    UnsafeKamikaze::throw_limit = 2;

    bool exception_caught = false;
    try {
        v.reserve(4);
    } catch (const std::runtime_error&) {
        exception_caught = true;
    }

    EXPECT_TRUE(exception_caught);

    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v.capacity(), 2);
}

TEST_F(VectorMemoryTest, CopyAssignmentProvidesStrongExceptionGuarantee) {
    Vector<KamikazeObject> v1;
    for (int i = 0; i < 5; ++i) {
        v1.push_back(KamikazeObject());
    }

    Vector<KamikazeObject> v2;
    v2.push_back(KamikazeObject());
    v2.push_back(KamikazeObject());
    
    size_t old_size = v2.size();
    size_t old_capacity = v2.capacity();
    KamikazeObject* old_data_ptr = v2.data();

    KamikazeObject::set_throw_on_copy(3);

    bool exception_caught = false;
    try {
        v2 = v1;
    } catch (const std::runtime_error&) {
        exception_caught = true;
    }

    EXPECT_TRUE(exception_caught);
    
    EXPECT_EQ(v1.size(), 5);
    
    EXPECT_EQ(v2.size(), old_size);
    EXPECT_EQ(v2.capacity(), old_capacity);
    EXPECT_EQ(v2.data(), old_data_ptr);
}
