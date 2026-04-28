#pragma once

#include "axioms.h"

using namespace cosmology;

///////////////////
////// constants
///////////
#define RENDER_WIDTH 300
#define RENDER_HEIGHT 1

#define SPI_DEVICE spi0
#define SPI_MOSI_PIN 16
#define SPI_CHIP_SELECT_PIN 17
#define SPI_CLOCK_PIN 18
#define SPI_MISO_PIN 19

#define HEADER_SIZE 8
#define LED_COUNT (RENDER_WIDTH * RENDER_HEIGHT)
#define BAUDRATE (8 * 1000 * 1000)

#define WS2812_HAS_W false
#define WS2812_PIN 2

namespace quetzal {

}
