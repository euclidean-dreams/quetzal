#include <iostream>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "generated/ws2812.pio.h"

// definitions
/////////////////
#define SPI_DEVICE spi0
#define SPI_MOSI_PIN 16
#define SPI_CHIP_SELECT_PIN 17
#define SPI_CLOCK_PIN 18
#define SPI_MISO_PIN 19

#define HEADER_SIZE 8
#define LED_COUNT (8 * 32)
#define SPI_PACKET_SIZE (HEADER_SIZE + LED_COUNT * 3)
#define BAUDRATE (8 * 1000 * 1000)

#define WS2812_HAS_W false
#define WS2812_PIN 2


// spi
/////////
uint8_t transmit_buffer[SPI_PACKET_SIZE];
uint8_t receive_buffer[SPI_PACKET_SIZE];

static void initialize_spi() {
    spi_init(SPI_DEVICE, BAUDRATE);
    spi_set_slave(SPI_DEVICE, true);
    spi_set_format(SPI_DEVICE, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_set_function(SPI_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_CHIP_SELECT_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_CLOCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MISO_PIN, GPIO_FUNC_SPI);
    std::cout << "spi ready" << std::endl;
}

static bool spi_header_is_valid() {
    if (receive_buffer[0] == 1 &&
        receive_buffer[1] == 2 &&
        receive_buffer[2] == 4 &&
        receive_buffer[3] == 8 &&
        receive_buffer[4] == 7 &&
        receive_buffer[5] == 5) {
        return true;
    } else {
        return false;
    }
}


// primitive
///////////////
class RGBColor {
public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    RGBColor(uint8_t red, uint8_t green, uint8_t blue) : red{red}, green{green}, blue{blue} {}

    uint32_t serialize() {
        return ((uint32_t) (green) << 16) | ((uint32_t) (red) << 8) | (uint32_t) (blue);
    }
};


// ws2812
////////////
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static void initialize_ws2812() {
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, WS2812_HAS_W);
    std::cout << "ws2812 ready" << std::endl;

    for (int i = 0; i < LED_COUNT; i++) {
        put_pixel(RGBColor(0, 0, 0).serialize());
    }
    sleep_ms(10);

    for (int i = 0; i < 2; i++) {
        put_pixel(RGBColor(0, 0, 0).serialize());
    }
    for (int i = 0; i < 2; i++) {
        put_pixel(RGBColor(0, 33, 77).serialize());
    }
    for (int i = 0; i < 2; i++) {
        put_pixel(RGBColor(0, 77, 33).serialize());
    }
    sleep_ms(10);
    std::cout << "ws2812 showing test pattern" << std::endl;
}


// main
//////////
int main() {
    stdio_init_all();
    std::cout << "(~) quetzal init..." << std::endl;

    initialize_spi();
    initialize_ws2812();

    while (true) {
        spi_write_read_blocking(SPI_DEVICE, transmit_buffer, receive_buffer, SPI_PACKET_SIZE);
        if (spi_header_is_valid()) {
            for (int i = HEADER_SIZE; i < SPI_PACKET_SIZE; i += 3) {
                auto color = RGBColor(receive_buffer[i],
                                      receive_buffer[i + 1],
                                      receive_buffer[i + 2]);
                auto serialized_color = color.serialize();
                put_pixel(serialized_color);
            }
        } else {
            std::cout << "encountered invalid spi header, re-initializing spi..." << std::endl;
            // the spi hardware will happily begin reading halfway through a transmission, as well as other nonsense
            // if we encounter a transmission without a valid header, drop it and reset the SPI
            initialize_spi();
        }
    }

}
