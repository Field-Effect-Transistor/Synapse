# /plugin/PrinterLib/CMakeLists.txt

add_library(PrinterLib_Objects OBJECT
    src/ASTPrinter.cpp
    src/ASTTreePrinter.cpp
    src/StringValue.cpp
    src/PrinterLibPlugin.cpp
)

target_link_libraries(PrinterLib_Objects PUBLIC SynapseCore SynapseCompilerFlags)
target_include_directories(PrinterLib_Objects PUBLIC src)

add_library(PrinterLib MODULE $<TARGET_OBJECTS:PrinterLib_Objects>)
target_link_libraries(PrinterLib PRIVATE SynapseCompilerFlags)

set_target_properties(PrinterLib PROPERTIES
    PREFIX ""
    DEBUG_POSTFIX "_d"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugin
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugin
)

if (TARGET GTest::gtest_main)
    add_executable(PrinterLib_Tests 
        tests/test_ast_tree_printer.cpp
        tests/test_string_value.cpp 
        tests/test_ast_printer.cpp
        tests/test_plugin.cpp
    )
    
    target_link_libraries(PrinterLib_Tests PRIVATE 
        PrinterLib_Objects
        GTest::gtest_main
        SynapseCompilerFlags
    )
    
    add_dependencies(PrinterLib_Tests PrinterLib)
    
    include(GoogleTest)
    gtest_discover_tests(PrinterLib_Tests)

    set_property(GLOBAL APPEND PROPERTY SYNAPSE_PLUGIN_TESTS PrinterLib_Tests)
endif()
