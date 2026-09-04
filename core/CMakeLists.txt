# /core/CMakeLists.txt

add_library(SynapseCore STATIC
    src/Lexer.cpp
    src/Parser.cpp
    src/Value.cpp
    src/Token.cpp
    src/MathEvaluator.cpp
    src/SymbolTable.cpp
    src/EnvironmentManager.cpp
    src/PluginRegistry.cpp
    src/Calculator.cpp
    src/DynamicLibrary.cpp
    src/SynapseEngine.cpp
    src/ExecutionContext.cpp
    src/plugins/StdLibPlugin.cpp
)

target_include_directories(SynapseCore PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(SynapseCore PUBLIC SynapseUtils SynapseCompilerFlags)
