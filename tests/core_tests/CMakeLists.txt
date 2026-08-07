# /tests/core_tests/CMakeLists.txt

add_executable(SynapseCoreTests
    test_lexer.cpp
    test_value.cpp    
    test_ast.cpp
    test_ast_tree_printer.cpp
    test_parser.cpp
    test_callable.cpp
    test_context.cpp
)

target_link_libraries(SynapseCoreTests
    PRIVATE
    SynapseCore
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(SynapseCoreTests)
