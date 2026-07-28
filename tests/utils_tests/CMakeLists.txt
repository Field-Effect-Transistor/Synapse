# /tests/utils_tests/CMakeLists.txt

add_executable(HermesUtilsTests test_unique_ptr.cpp test_arena_allocator.cpp)

target_link_libraries(HermesUtilsTests
    PRIVATE
    HermesUtils
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(HermesUtilsTests)
