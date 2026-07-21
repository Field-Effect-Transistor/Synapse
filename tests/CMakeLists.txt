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

find_program(LCOV_PATH lcov)
find_program(GENHTML_PATH genhtml)

if(LCOV_PATH AND GENHTML_PATH)
    add_custom_target(coverage
        # Крок А: Спочатку обов'язково запускаємо тести!
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure

        # Крок Б: Збираємо покриття
        COMMAND ${LCOV_PATH} --capture 
                --directory ${CMAKE_BINARY_DIR} 
                --output-file coverage.info 
                --ignore-errors inconsistent 
                --rc branch_coverage=1

        # Крок В: Фільтруємо системні файли та самі тести
        COMMAND ${LCOV_PATH} --remove coverage.info 
                "'/usr/*'" 
                "'*/tests/*'" 
                "'*/_deps/*'" 
                --output-file coverage_filtered.info 
                --ignore-errors inconsistent 
                --ignore-errors unused 
                --rc branch_coverage=1

        # Крок Г: Генеруємо HTML-звіт (використовуємо CMAKE_SOURCE_DIR для префікса)
        COMMAND ${GENHTML_PATH} coverage_filtered.info 
                --output-directory coverage_report 
                --ignore-errors inconsistent 
                --prefix ${CMAKE_SOURCE_DIR} 
                --rc branch_coverage=1

        # Крок Д: Виводимо зручне повідомлення в консоль
        COMMAND ${CMAKE_COMMAND} -E echo "✅ Звіт згенеровано: ${CMAKE_BINARY_DIR}/coverage_report/index.html"

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS HermesTests
        COMMENT "Запуск тестів та генерація HTML-звіту покриття..."
    )
else()
    message(WARNING "Утиліти lcov або genhtml не знайдені! Таргет 'coverage' не створено.")
endif()
