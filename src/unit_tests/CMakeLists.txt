# Unit tests executable
add_executable(test_boyo
    compiler/compiler_tests.cpp
    lexer/lexer_tests.cpp
    statement/statement_tests.cpp
    parser/parser_tests.cpp
    expression/expression_tests.cpp
)

target_link_libraries(test_boyo PRIVATE
    compiler
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_boyo)
