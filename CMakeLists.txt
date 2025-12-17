cmake_minimum_required(VERSION 3.21)
project(boyo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find cpp-cli from Conan
find_package(cpp-cli REQUIRED)

# Find GTest from Conan (transitive dependency via cpp-cli)
find_package(GTest REQUIRED)

# Enable testing
enable_testing()

# Add compiler library
add_subdirectory(src/compiler)

# Add boyo executable
add_executable(boyo
    src/main.cpp
)

target_link_libraries(boyo PRIVATE
    cpp-cli::core_cli
    compiler
)

# Function to create symlink in project root for Debug builds
function(symlink_tool_to_root TOOL_NAME)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(TARGET ${TOOL_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
                $<TARGET_FILE:${TOOL_NAME}>
                ${CMAKE_SOURCE_DIR}/${TOOL_NAME}
            COMMENT "Creating symlink: ${CMAKE_SOURCE_DIR}/${TOOL_NAME}"
        )
    endif()
endfunction()

symlink_tool_to_root(boyo)

# Add unit tests
add_subdirectory(src/unit_tests)

# Create symlink for compile_commands.json in project root for clangd
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
    )
endif()

