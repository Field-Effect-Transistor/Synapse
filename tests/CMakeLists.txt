# /tests/CMakeLists.txt

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(HermesTests test_lexer.cpp)

target_link_libraries(HermesTests
    PRIVATE
    HermesCore
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(HermesTests)
