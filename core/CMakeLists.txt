# /core/src/CMakeLists.txt

add_library(SynapseCore src/Lexer.cpp src/Parser.cpp)

target_include_directories(SynapseCore PUBLIC 
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(SynapseCore PUBLIC SynapseUtils)
