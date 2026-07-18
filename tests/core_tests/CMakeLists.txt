# /tests/core_tests/CMakeLists.txt

add_executable(HermesCoreTests test_lexer.cpp)

target_link_libraries(HermesCoreTests
    PRIVATE
    HermesCore
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(HermesCoreTests)
