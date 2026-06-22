cmake_minimum_required(VERSION 4.3.4)

project(Sena LANGUAGES C)

add_executable(Sena)

target_sources(Sena PRIVATE
    src/main.c
    src/editor/core/buffer.c
)

target_include_directories(Sena PRIVATE
    include
)

target_compile_options(Sena PRIVATE
    -fsanitize=address
    -g
)

target_link_options(Sena PRIVATE
    -fsanitize=address
)