# Compiler library
add_library(compiler STATIC
    compiler/compiler.cpp
    lexer/lexer.cpp
    parser/parser.cpp
    statement/statement.cpp
    statement/expression.cpp
    utils/code_printer.cpp
)

target_include_directories(compiler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/compiler/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer/include
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/include
    ${CMAKE_CURRENT_SOURCE_DIR}/statement/include
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/include
)

target_compile_features(compiler PUBLIC cxx_std_20)
