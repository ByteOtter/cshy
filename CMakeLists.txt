cmake_minimum_required(VERSION 3.2)
project(shy C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FindPkgConfig)
pkg_check_modules(GCRYPT IMPORTED_TARGET libgcrypt)
add_executable(cshy
  src/main.c
  src/shy_file.c
  )
target_include_directories(cshy PUBLIC include/)
target_link_libraries(cshy PUBLIC zstd PkgConfig::GCRYPT)
