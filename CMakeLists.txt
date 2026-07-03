cmake_minimum_required(VERSION 3.19)

include(pico_sdk_import.cmake)
project(quetzal C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#############
## options ##
#############

# pick one!
option(dmx "build for dmx" OFF)
option(keyhole "build for keyholes" OFF)
option(sigurd "build for sigurd" OFF)
message("dmx: ${dmx}, keyhole: ${keyhole}, sigurd: ${sigurd}")

set(RENDER_WIDTH 64 CACHE STRING "width of the output frame")
set(RENDER_HEIGHT 64 CACHE STRING "height of the output frame")
add_compile_definitions(COMPILED_RENDER_WIDTH=${RENDER_WIDTH})
add_compile_definitions(COMPILED_RENDER_HEIGHT=${RENDER_HEIGHT})


if (dmx)
    # set automatically by the browser
    add_compile_definitions(DMX)
elseif (keyhole)
    add_compile_definitions(KEYHOLE)
elseif (sigurd)
    add_compile_definitions(SIGURD)
else ()
    message(FATAL_ERROR "pick one of -Ddmx=ON -Dkeyhole=ON or -Dsigurd=ON")
endif ()

pico_sdk_init()

include(lib/Pico-DMX/interfaceLibForPicoSDK.cmake)

set(
        SOURCE_FILES
        src/main.cpp
        src/lantern.h
        src/paradigm.h
        src/quetzal.h
        src/spi_connection.h
        src/sigin.h
)

add_executable(quetzal ${SOURCE_FILES})
target_include_directories(quetzal PRIVATE src)

###############
## cosmology ##
###############
add_subdirectory(lib/cosmology)
target_link_libraries(quetzal PRIVATE cosmology)
target_link_libraries(quetzal PRIVATE cosmology_selections)
add_compile_definitions(FFT_FRAME_SIZE=128)
add_compile_definitions(FFT_WINDOW_SIZE=1)


target_link_libraries(quetzal PRIVATE pico_stdlib hardware_spi hardware_pio hardware_adc hardware_dma picodmx pico_multicore)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)

pico_generate_pio_header(quetzal ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_enable_stdio_usb(quetzal 1)
pico_enable_stdio_uart(quetzal 0)

pico_add_extra_outputs(quetzal)
