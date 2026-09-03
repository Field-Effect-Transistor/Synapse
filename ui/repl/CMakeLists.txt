#   /ui/repl/CMakeLists.txt

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

add_executable(synapse-repl
    src/main.cpp
    src/StringUtils.cpp
    src/CommandDispatcher.cpp
    src/WorkspaceManager.cpp
    src/ReplApp.cpp
)

target_include_directories(synapse-repl PRIVATE include)
target_link_libraries(synapse-repl PRIVATE
    SynapseCore
    nlohmann_json::nlohmann_json
)
