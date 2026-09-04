# /tests/core_tests/CMakeLists.txt

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy_empty_lib.cpp "int dummy_symbol = 42;")
add_library(DummyEmptyLib MODULE ${CMAKE_CURRENT_BINARY_DIR}/dummy_empty_lib.cpp)
set_target_properties(DummyEmptyLib PROPERTIES PREFIX "")

add_executable(SynapseCoreTests
    test_lexer.cpp
    test_value.cpp    
    test_ast.cpp
    test_parser.cpp
    test_callable.cpp
    test_math_evaluator.cpp
    test_plugin_registry.cpp
    test_calculator.cpp
    test_synapse_engine.cpp
    test_memory_model.cpp
    test_environment_manager.cpp
    test_dynamic_library.cpp
)

target_link_libraries(SynapseCoreTests
    PRIVATE
    SynapseCore
    GTest::gtest_main
    SynapseCompilerFlags
)

target_compile_definitions(SynapseCoreTests PRIVATE
    PRINTER_LIB_PATH="$<TARGET_FILE:PrinterLib>"
    EMPTY_LIB_PATH="$<TARGET_FILE:DummyEmptyLib>"
)

add_dependencies(SynapseCoreTests PrinterLib DummyEmptyLib)

include(GoogleTest)
gtest_discover_tests(SynapseCoreTests)
