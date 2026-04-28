#include "paradigm.h"
#include <iostream>

#include "cosmology.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/pio.h"
#include "DmxOutput.h"
#include "lantern.h"

// dmx
/////////
DmxOutput dmx;
#define DMX_FRAME_LENGTH 512
uint8_t universe[DMX_FRAME_LENGTH + 1];

// spi
/////////
#ifdef DMX
#define SPI_PACKET_SIZE (HEADER_SIZE + DMX_FRAME_LENGTH)
#elifdef KEYHOLE
#define SPI_PACKET_SIZE (HEADER_SIZE + LED_COUNT * 3)
#endif

uint8_t previous_header_index = -1;

uint8_t transmit_buffer[SPI_PACKET_SIZE];
uint8_t receive_buffer[SPI_PACKET_SIZE];

namespace quetzal {
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

// keyhole
/////////////
void keyhole_loop() {
    Lantern lantern{};
    std::cout << "ws2812 lantern ready" << std::endl;
    lantern.show_test_pattern();
    std::cout << "ws2812 lantern showing test pattern" << std::endl;

    while (true) {
        spi_write_read_blocking(SPI_DEVICE, transmit_buffer, receive_buffer, SPI_PACKET_SIZE);
        if (spi_header_is_valid()) {
            auto x = 0;
            auto y = 0;
            for (int i = HEADER_SIZE; i < SPI_PACKET_SIZE; i += 3) {
                auto color = Color{
                    receive_buffer[i],
                    receive_buffer[i + 1],
                    receive_buffer[i + 2]
                };
                lantern.lattice.set_pith(x, y, Pith{color});
                x++;
                if (x % RENDER_WIDTH == 0) {
                    x = 0;
                    y++;
                }
            }
            lantern.show();
        } else {
            std::cout << "encountered invalid spi header, re-initializing spi..." << std::endl;
            // the spi hardware will happily begin reading halfway through a transmission, as well as other nonsense
            // if we encounter a transmission without a valid header, drop it and reset the SPI
            initialize_spi();
        }
    }
}


// dmx
/////////////
void dmx_loop() {
    dmx.begin(0);
    for (int i = 1; i < DMX_FRAME_LENGTH + 1; i++) {
        universe[i] = 0;
    }

    while (true) {
        spi_write_read_blocking(SPI_DEVICE, transmit_buffer, receive_buffer, SPI_PACKET_SIZE);
        if (spi_header_is_valid()) {
            for (int i = 1; i < DMX_FRAME_LENGTH + 1; i++) {
                universe[i] = receive_buffer[i - 1 + HEADER_SIZE];
            }
            while (dmx.busy()) {
                /* Do nothing while the DMX frame transmits */
            }
            dmx.write(universe, DMX_FRAME_LENGTH);
        } else {
            std::cout << "encountered invalid spi header, re-initializing spi..." << std::endl;
            // the spi hardware will happily begin reading halfway through a transmission, as well as other nonsense
            // if we encounter a transmission without a valid header, drop it and reset the SPI
            initialize_spi();
        }
    }
}
}

using namespace quetzal;

// main
//////////
int main() {
    Cosmology cosmology{RENDER_WIDTH, RENDER_HEIGHT, 0, Impressions::workshop};

    stdio_init_all();
    std::cout << "(~) quetzal init..." << std::endl;

    initialize_spi();
    std::cout << "(~) spi init..." << std::endl;

#ifdef DMX
    dmx_loop();
#elifdef KEYHOLE
    keyhole_loop();
#endif
}
