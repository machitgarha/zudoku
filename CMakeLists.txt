cmake_minimum_required(VERSION 3.11)

project(Zudoku)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/app.cpp
    src/sudoku-solver.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    include/

    # Static libraries
    lib/flossy
    lib/rapidcsv/src
    lib/chop/include
)
