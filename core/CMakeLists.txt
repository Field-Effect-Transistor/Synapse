# /core/src/CMakeLists.txt

add_library(SynapseCore 
    src/Lexer.cpp 
    src/Parser.cpp
    src/Value.cpp
    src/Token.cpp
    src/MathEvaluator.cpp
    src/Context.cpp
    src/ContextManager.cpp
    src/PluginRegistry.cpp
    src/plugins/StdLibPlugin.cpp
)

target_include_directories(SynapseCore PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(SynapseCore PUBLIC SynapseUtils)
