cmake_minimum_required(VERSION 3.2)
project(shy C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(cshy
  src/main.c
  )
target_include_directories(cshy PUBLIC include/)
