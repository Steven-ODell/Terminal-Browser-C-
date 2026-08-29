cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Tells Clangd how to read your project
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(first_cmake_program
    main.cpp
)
