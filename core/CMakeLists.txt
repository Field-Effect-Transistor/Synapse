# /core/src/CMakeLists.txt

add_library(HermesCore src/Lexer.cpp)

target_include_directories(HermesCore PUBLIC 
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(HermesCore PUBLIC HermesUtils)
