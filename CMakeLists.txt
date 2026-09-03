# /CMakeLists.txt
cmake_minimum_required(VERSION 3.22)
project(Synapse LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include(CTest)

# Глобальне застосування інструментації для збірки з тестами
if(BUILD_TESTING)
    add_compile_options(--coverage -O0 -g -fsanitize=address)
    add_link_options(--coverage -fsanitize=address)
endif()

add_subdirectory(utils)
add_subdirectory(core)
add_subdirectory(ui)

if(BUILD_TESTING)
    add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()

add_subdirectory(plugin)

# ==============================================================================
# АВТОМАТИЗАЦІЯ COVERAGE (Таргет 'coverage')
# Розміщено тут, а не в tests/, бо на цей момент вже відома
# властивість SYNAPSE_PLUGIN_TESTS, зібрана плагінами.
# ==============================================================================
if(BUILD_TESTING)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)

    if(LCOV_PATH AND GENHTML_PATH)
        get_property(PLUGIN_TESTS GLOBAL PROPERTY SYNAPSE_PLUGIN_TESTS)

        add_custom_target(coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${LCOV_PATH} --capture
                    --directory .
                    --output-file coverage.info
                    --ignore-errors inconsistent
                    --rc branch_coverage=1
            COMMAND ${LCOV_PATH} --remove coverage.info
                    "'/usr/*'"
                    "'*/tests/*'"
                    "'*/_deps/*'"
                    --output-file coverage_filtered.info
                    --ignore-errors inconsistent
                    --ignore-errors unused
                    --rc branch_coverage=1
            COMMAND ${GENHTML_PATH} coverage_filtered.info
                    --output-directory coverage_report
                    --ignore-errors inconsistent
                    --prefix ${CMAKE_SOURCE_DIR}
                    --rc branch_coverage=1
            COMMAND ${CMAKE_COMMAND} -E echo "✅ Звіт згенеровано: ${CMAKE_BINARY_DIR}/coverage_report/index.html"

            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS SynapseCoreTests SynapseUtilsTests ${PLUGIN_TESTS}
            COMMENT "Запуск тестів та генерація HTML-звіту покриття..."
        )
    else()
        message(WARNING "Утиліти lcov або genhtml не знайдені! Таргет 'coverage' не створено.")
    endif()
endif()
