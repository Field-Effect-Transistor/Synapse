# /tests/utils_tests/CMakeLists.txt

add_executable(SynapseUtilsTests test_unique_ptr.cpp test_arena_allocator.cpp test_vector.cpp)

target_link_libraries(SynapseUtilsTests
    PRIVATE
    SynapseUtils
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(SynapseUtilsTests)
