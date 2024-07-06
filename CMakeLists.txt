cmake_minimum_required(VERSION 3.25)

include(pico_sdk_import.cmake)
project(quetzal C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()
set(
        SOURCE_FILES
        src/main.cpp
)

add_executable(quetzal ${SOURCE_FILES})
target_include_directories(quetzal PRIVATE src)
target_link_libraries(quetzal pico_stdlib hardware_spi hardware_pio)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)

pico_generate_pio_header(quetzal ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

pico_add_extra_outputs(quetzal)

