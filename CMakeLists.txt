cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Tells Clangd how to read your project
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(Cexp
    main.cpp
    term_set.h
    term_set.cpp
    state_start.h
    state_start.cpp
)
