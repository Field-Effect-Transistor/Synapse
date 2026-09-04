# /CMakeLists.txt
cmake_minimum_required(VERSION 3.22)
project(Synapse LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# === ОПЦІЇ ПРОЕКТУ ===
option(ENABLE_ASAN      "Enable Address Sanitizer for Synapse targets"  OFF)
option(ENABLE_COVERAGE  "Enable Coverage reporting for Synapse targets" OFF)
option(BUILD_TESTING    "Build the testing tree"                        OFF)

include(CTest)

add_library(SynapseCompilerFlags INTERFACE)

if(ENABLE_ASAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(SynapseCompilerFlags INTERFACE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(SynapseCompilerFlags INTERFACE -fsanitize=address)
    elseif(MSVC)
        target_compile_options(SynapseCompilerFlags INTERFACE /fsanitize=address)
    endif()
endif()

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(SynapseCompilerFlags INTERFACE --coverage -O0 -g)
        target_link_options(SynapseCompilerFlags INTERFACE --coverage)
    else()
        message(WARNING "Coverage is only supported on GNU/Clang compilers.")
    endif()
endif()

add_subdirectory(utils)
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(plugin)

if(BUILD_TESTING)
    add_subdirectory(tests)

    if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)

        if(LCOV_PATH AND GENHTML_PATH)
            get_property(PLUGIN_TESTS GLOBAL PROPERTY SYNAPSE_PLUGIN_TESTS)

            add_custom_target(coverage
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                COMMAND ${LCOV_PATH} --capture --base-directory ${CMAKE_SOURCE_DIR} --directory ${CMAKE_BINARY_DIR} --output-file coverage.info --ignore-errors inconsistent --rc branch_coverage=1
                COMMAND ${LCOV_PATH} --remove coverage.info "'/usr/*'" "'*/tests/*'" "'*/_deps/*'" --output-file coverage_filtered.info --ignore-errors inconsistent --ignore-errors unused --rc branch_coverage=1
                COMMAND ${GENHTML_PATH} coverage_filtered.info --output-directory coverage_report --ignore-errors inconsistent --prefix ${CMAKE_SOURCE_DIR} --rc branch_coverage=1
                COMMAND ${CMAKE_COMMAND} -E echo "✅ Звіт згенеровано: ${CMAKE_BINARY_DIR}/coverage_report/index.html"
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                DEPENDS SynapseCoreTests SynapseUtilsTests ${PLUGIN_TESTS}
            )
        else()
            message(WARNING "lcov or genhtml not found! The 'coverage' target will not be created. Please install lcov.")
        endif()
    endif()
endif()
