include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# One executable per test file
add_executable(smoke_tests smoke_tests.cpp)
target_link_libraries(smoke_tests PRIVATE compiler_lib GTest::gtest_main)
gtest_discover_tests(smoke_tests)

add_executable(parser_tests parser_tests.cpp)
target_link_libraries(parser_tests PRIVATE compiler_lib GTest::gtest_main)
gtest_discover_tests(parser_tests)
