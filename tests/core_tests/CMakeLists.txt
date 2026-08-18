# /tests/core_tests/CMakeLists.txt

add_executable(SynapseCoreTests
    test_lexer.cpp
    test_value.cpp    
    test_ast.cpp
    test_parser.cpp
    test_callable.cpp
    test_context.cpp
    test_context_manager.cpp
    test_math_evaluator.cpp
    test_plugin_registry.cpp
    test_calculator.cpp
)

target_link_libraries(SynapseCoreTests
    PRIVATE
    SynapseCore
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(SynapseCoreTests)
