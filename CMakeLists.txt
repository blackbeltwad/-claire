cmake_minimum_required(VERSION 3.20)

project(-claire LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic)

add_library(compiler_lib
  src/frontend/lexer.cpp
  src/frontend/parser.cpp
)
target_include_directories(compiler_lib PUBLIC include)

add_executable(claire src/main.cpp)
target_link_libraries(claire PRIVATE compiler_lib)

enable_testing()
add_subdirectory(tests)
