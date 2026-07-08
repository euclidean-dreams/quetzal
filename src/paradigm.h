#pragma once

#include "axioms.h"

using namespace cosmology;

///////////////////
////// constants
///////////
#define RENDER_WIDTH COMPILED_RENDER_WIDTH
#define RENDER_HEIGHT COMPILED_RENDER_HEIGHT

#define LED_COUNT (RENDER_WIDTH * RENDER_HEIGHT)

#define DEBUG false
#define WS2812_PIN 1

#define SPI_DEVICE spi0
#define SPI_MOSI_PIN 20
#define SPI_CHIP_SELECT_PIN 17
#define SPI_CLOCK_PIN 18
#define SPI_MISO_PIN 19
#define BAUDRATE (8 * 1000 * 1000)

#define HEADER_SIZE 16

#ifdef DMX
#define SPI_PACKET_SIZE (HEADER_SIZE + DMX_FRAME_LENGTH)
#elifdef KEYHOLE
#define SPI_PACKET_SIZE (HEADER_SIZE + LED_COUNT * 3)
#elifdef SIGURD
#define SPI_PACKET_SIZE 0
#endif

namespace quetzal {

}
