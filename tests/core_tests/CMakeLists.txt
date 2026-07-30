# /tests/core_tests/CMakeLists.txt

add_executable(HermesCoreTests
    test_lexer.cpp
    test_value.cpp    
    test_ast.cpp
    test_ast_tree_printer.cpp
)

target_link_libraries(HermesCoreTests
    PRIVATE
    HermesCore
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(HermesCoreTests)
