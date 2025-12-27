# Examples directory

# Code printer demo
add_executable(code_printer_demo
    code_printer_demo.cpp
)

target_link_libraries(code_printer_demo PRIVATE
    compiler
)

# Pipeline demo
add_executable(pipeline_demo
    pipeline_demo.cpp
)

target_link_libraries(pipeline_demo PRIVATE
    compiler
)

# Create symlinks for demo executables
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET code_printer_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            $<TARGET_FILE:code_printer_demo>
            ${CMAKE_SOURCE_DIR}/code_printer_demo
        COMMENT "Creating symlink: ${CMAKE_SOURCE_DIR}/code_printer_demo"
    )
    
    add_custom_command(TARGET pipeline_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            $<TARGET_FILE:pipeline_demo>
            ${CMAKE_SOURCE_DIR}/pipeline_demo
        COMMENT "Creating symlink: ${CMAKE_SOURCE_DIR}/pipeline_demo"
    )
endif()

