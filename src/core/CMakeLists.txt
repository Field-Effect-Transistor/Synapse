#   /src/core/CMakeLists.txt
add_executable(${PROJECT_NAME} main.cpp Lexer.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
