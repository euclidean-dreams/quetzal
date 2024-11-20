#include <iostream>
#include <vector>
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
#define RENDER_WIDTH 277
#define RENDER_HEIGHT 1
#define LED_COUNT (RENDER_WIDTH * RENDER_HEIGHT)
#define SPI_PACKET_SIZE (HEADER_SIZE + LED_COUNT * 3)
#define BAUDRATE (8 * 1000 * 1000)

#define WS2812_HAS_W false
#define WS2812_PIN 2


// spi
/////////
uint8_t previous_header_index = -1;

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
        uint8_t header_index = receive_buffer[6];
        auto expected_header_index = previous_header_index;
        expected_header_index++;
        expected_header_index %= 256;
        if (header_index != expected_header_index) {
            printf("\nheader index mismatch - expected: %02x actual: %02x\n", expected_header_index, header_index);
        }
        previous_header_index = header_index;
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
std::vector<std::vector<RGBColor>> lattice;

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static void initialize_ws2812() {
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, WS2812_HAS_W);

    lattice = {};
    lattice.resize(RENDER_WIDTH);
    for (auto &row: lattice) {
        row.resize(RENDER_HEIGHT, {0, 0, 0});
    };
    std::cout << "ws2812 ready" << std::endl;

    for (int i = 0; i < LED_COUNT; i++) {
        put_pixel(RGBColor(0, 0, 0).serialize());
    }
    sleep_ms(10);
    for (int i = 0; i < LED_COUNT; i++) {
        auto size = 10;
        auto start_index = i - size / 2;
        auto end_index = i + size / 2;

        for (int j = 0; j < start_index; j++) {
            put_pixel(RGBColor(0, 0, 0).serialize());
        }
        for (int j = start_index; j < end_index && j < LED_COUNT; j++) {
            put_pixel(RGBColor(0, 33, 77).serialize());
        }
        for (int j = end_index; j < LED_COUNT; j++) {
            put_pixel(RGBColor(0, 0, 0).serialize());
        }
        sleep_ms(10);
    }
    for (int i = LED_COUNT; i > 0; i--) {
        auto size = 10;
        auto start_index = i - size / 2;
        auto end_index = i + size / 2;

        for (int j = 0; j < start_index; j++) {
            put_pixel(RGBColor(0, 0, 0).serialize());
        }
        for (int j = start_index; j < end_index && j < LED_COUNT; j++) {
            put_pixel(RGBColor(0, 33, 77).serialize());
        }
        for (int j = end_index; j < LED_COUNT; j++) {
            put_pixel(RGBColor(0, 0, 0).serialize());
        }
        sleep_ms(10);
    }
    std::cout << "ws2812 showing test pattern" << std::endl;
}

static void render_lattice() {
    for (int x = RENDER_WIDTH - 1; x >= 0; x--) {
        if (x % 2 != 0) {
            for (int y = 0; y < RENDER_HEIGHT; y++) {
                auto color = lattice[x][y];
                auto serialized_color = color.serialize();
                put_pixel(serialized_color);
            }
        } else {
            for (int y = RENDER_HEIGHT - 1; y >= 0; y--) {
                auto color = lattice[x][y];
                auto serialized_color = color.serialize();
                put_pixel(serialized_color);
            }
        }
    }
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
            auto x = 0;
            auto y = 0;
            for (int i = HEADER_SIZE; i < SPI_PACKET_SIZE; i += 3) {
                auto color = RGBColor(receive_buffer[i],
                                      receive_buffer[i + 1],
                                      receive_buffer[i + 2]);
                lattice[x][y] = color;
                x++;
                if (x % RENDER_WIDTH == 0) {
                    x = 0;
                    y++;
                }
            }
            render_lattice();
        } else {
            std::cout << "encountered invalid spi header, re-initializing spi..." << std::endl;
            // the spi hardware will happily begin reading halfway through a transmission, as well as other nonsense
            // if we encounter a transmission without a valid header, drop it and reset the SPI
            initialize_spi();
        }
    }

}
