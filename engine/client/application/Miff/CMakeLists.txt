
cmake_minimum_required(VERSION 2.8)

project(Miff)

# todo: if you're running a windows build, comment this out
# windows sucks, I don't care
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-z,norelro,-O3,--sort-common,--as-needed,--no-relax,-z,combreloc,-z,global,--no-omagic")

add_subdirectory(src)
