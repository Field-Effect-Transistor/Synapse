# plugin/PrinterLib/CMakeLists.txt
add_library(PrinterLib_Objects OBJECT
    src/ASTPrinter.cpp
    src/ASTTreePrinter.cpp
    src/StringValue.cpp
    src/PrinterLibPlugin.cpp
)

target_link_libraries(PrinterLib_Objects PUBLIC SynapseCore)
target_include_directories(PrinterLib_Objects PUBLIC src)

# Створюємо модуль тільки з об'єктних файлів (щоб не компілювати двічі)
add_library(PrinterLib MODULE $<TARGET_OBJECTS:PrinterLib_Objects>)

# === ВИПРАВЛЕННЯ: Кладемо плагін у папку bin/plugin ===
set_target_properties(PrinterLib PROPERTIES
    PREFIX ""
    DEBUG_POSTFIX "_d"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugin
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugin
    EXCLUDE_FROM_ALL TRUE
)

if (TARGET GTest::gtest_main)
    add_executable(PrinterLib_Tests 
        tests/test_ast_tree_printer.cpp
        tests/test_string_value.cpp 
    )
    
    target_link_libraries(PrinterLib_Tests PRIVATE 
        PrinterLib_Objects
        GTest::gtest_main
    )
    
    include(GoogleTest)
    gtest_discover_tests(PrinterLib_Tests)

    set_property(GLOBAL APPEND PROPERTY SYNAPSE_PLUGIN_TESTS PrinterLib_Tests)
endif()
