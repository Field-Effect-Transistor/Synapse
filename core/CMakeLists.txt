# /core/src/CMakeLists.txt

add_library(SynapseCore 
    src/Lexer.cpp 
    src/Parser.cpp
    src/Value.cpp
    src/Token.cpp
    src/MathEvaluator.cpp
)

target_include_directories(SynapseCore PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/synapse
    ${CMAKE_CURRENT_SOURCE_DIR}/include/internal
)

target_link_libraries(SynapseCore PUBLIC SynapseUtils)
