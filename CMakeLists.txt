cmake_minimum_required(VERSION 3.19)

include(pico_sdk_import.cmake)
project(quetzal C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# pick one!
option(dmx "build for dmx" OFF)
option(keyhole "build for keyholes" OFF)
message("dmx: ${dmx}, keyhole: ${keyhole}")

###############
## cosmology ##
###############
add_subdirectory(lib/cosmology)

if (dmx)
    # set automatically by the browser
    add_compile_definitions(DMX)
elseif (keyhole)
    add_compile_definitions(KEYHOLE)
else()
    message(FATAL_ERROR "pick either -Ddmx=ON for dmx, or -Dkeyhole=ON for an led matrix")
endif ()

pico_sdk_init()

include(lib/Pico-DMX/interfaceLibForPicoSDK.cmake)

set(
        SOURCE_FILES
        src/main.cpp
        src/lantern.h
        src/paradigm.h
)

add_executable(quetzal ${SOURCE_FILES})
target_include_directories(quetzal PRIVATE src)
target_link_libraries(quetzal pico_stdlib hardware_spi hardware_pio hardware_dma picodmx cosmology cosmology_selections)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)

pico_generate_pio_header(quetzal ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

pico_add_extra_outputs(quetzal)

