# Compiler library
add_library(compiler STATIC
    compiler.cpp
    parser.cpp
    statement.cpp
)

target_include_directories(compiler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(compiler PUBLIC cxx_std_23)


