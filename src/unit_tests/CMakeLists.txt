# Unit tests executable
add_executable(test_boyo
    compiler_tests.cpp
    statement_tests.cpp
    parser_tests.cpp
)

target_link_libraries(test_boyo PRIVATE
    compiler
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_boyo)


