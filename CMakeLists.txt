cmake_minimum_required(VERSION 3.20)

project(-claire LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(claire
  src/main.cpp
  src/frontend/lexer.cpp
  src/frontend/parser.cpp
)

target_include_directories(claire PRIVATE include)

