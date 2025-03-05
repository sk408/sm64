cmake_minimum_required(VERSION 3.13)

add_library(g722_codec STATIC
    g722_encode.c
    g722_decode.c
)

target_include_directories(g722_codec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}) 