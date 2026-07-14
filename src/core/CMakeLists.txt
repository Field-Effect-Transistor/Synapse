# /src/core/CMakeLists.txt

add_library(HermesCore Lexer.cpp)

target_include_directories(HermesCore PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR} 
)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE HermesCore)
