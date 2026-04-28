#pragma once
#include "paradigm.h"
#include "hardware/spi.h"
#include "hardware/pio.h"

#define SPI_DEVICE spi0
#ifdef DMX
#define SPI_PACKET_SIZE (HEADER_SIZE + DMX_FRAME_LENGTH)
#elifdef KEYHOLE
#define SPI_PACKET_SIZE (HEADER_SIZE + LED_COUNT * 3)
#endif


namespace quetzal {
class SPIConnection : public Name {
public:
    uint8_t transmit_buffer[SPI_PACKET_SIZE];
    uint8_t receive_buffer[SPI_PACKET_SIZE];
    uint8_t previous_header_index = -1;

    SPIConnection() {
        spi_init(SPI_DEVICE, BAUDRATE);
        spi_set_slave(SPI_DEVICE, true);
        spi_set_format(SPI_DEVICE, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
        gpio_set_function(SPI_MOSI_PIN, GPIO_FUNC_SPI);
        gpio_set_function(SPI_CHIP_SELECT_PIN, GPIO_FUNC_SPI);
        gpio_set_function(SPI_CLOCK_PIN, GPIO_FUNC_SPI);
        gpio_set_function(SPI_MISO_PIN, GPIO_FUNC_SPI);
    }

    ~SPIConnection() {
        spi_deinit(SPI_DEVICE);
    }

    bool spi_header_is_valid() {
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

    void exchange() {
        spi_write_read_blocking(SPI_DEVICE, transmit_buffer, receive_buffer, SPI_PACKET_SIZE);
    }
};
}
