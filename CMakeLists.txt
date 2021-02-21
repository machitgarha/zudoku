cmake_minimum_required(VERSION 3.11)

project(Zudoku)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/zudoku.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    include/

    # Libraries
    lib/flossy
)
