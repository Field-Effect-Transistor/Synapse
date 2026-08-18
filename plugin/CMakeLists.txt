# /plugin/CMakeLists.txt

# CONFIGURE_DEPENDS гарантує, що CMake сам перегенерує файли, якщо додати нову папку плагіна
file(GLOB PLUGIN_ENTRIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*")

foreach(ENTRY ${PLUGIN_ENTRIES})
    if(IS_DIRECTORY "${ENTRY}" AND EXISTS "${ENTRY}/CMakeLists.txt")
        add_subdirectory("${ENTRY}")
    endif()
endforeach()
