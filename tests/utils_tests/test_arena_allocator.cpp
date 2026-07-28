//  /tests/utils_tests/test_arena_allocator.cpp
#include <gtest/gtest.h>
#include "ArenaAllocator.hpp"

using namespace Hermes;

TEST(ArenaAllocatorTest, BasicAllocation) {
    ArenaAllocator arena(1024);
    
    void* ptr1 = arena.allocate(10);
    EXPECT_NE(ptr1, nullptr);

    int* ptr2 = arena.allocate<int>(); 
    EXPECT_NE(ptr2, nullptr);
}

TEST(ArenaAllocatorTest, Alignment) {
    ArenaAllocator arena(1024);
    
    void* p1 = arena.allocate(1);
    
    void* p2 = arena.allocate(sizeof(double));
    
    uintptr_t address = reinterpret_cast<uintptr_t>(p2);
    size_t align = alignof(std::max_align_t);
    
    EXPECT_EQ(address % align, 0); 
}

TEST(ArenaAllocatorTest, OutOfMemoryThrows) {
    ArenaAllocator arena(16);
    
    void* p1 = arena.allocate(8);
    EXPECT_NE(p1, nullptr);

    // Просимо ще 10 байтів (8 + 10 > 16). Має вилетіти виняток!
    EXPECT_THROW(arena.allocate(10), std::bad_alloc);
}

TEST(ArenaAllocatorTest, ResetClearsOffset) {
    ArenaAllocator arena(16);
    
    // Заповнюємо Арену повністю
    arena.allocate(16);
    EXPECT_THROW(arena.allocate(1), std::bad_alloc);
    
    // Скидаємо
    arena.reset();
    
    // Тепер має знову вистачити місця!
    void* p = arena.allocate(16);
    EXPECT_NE(p, nullptr);
}

TEST(ArenaAllocatorTest, MoveSemantics) {
    ArenaAllocator arena1(1024);
    arena1.allocate(100);

    // Переміщуємо arena1 в arena2
    ArenaAllocator arena2(std::move(arena1));

    // Після переміщення arena1 має стати порожньою (capacity = 0)
    // Тобто спроба виділити навіть 1 байт в arena1 призведе до помилки
    EXPECT_THROW(arena1.allocate(1), std::bad_alloc);

    // А arena2 повинна чудово працювати і мати залишок пам'яті
    void* p2 = arena2.allocate(100);
    EXPECT_NE(p2, nullptr);
}
