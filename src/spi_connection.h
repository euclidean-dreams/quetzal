#pragma once
#include "paradigm.h"
#include "hardware/spi.h"
#include "hardware/pio.h"

namespace quetzal {
class SPIConnection : public Name {
public:
    vect<uint8_t> transmission;
    vect<uint8_t> reception;
    int packet_size;
    uint8_t previous_header_index = -1;

    SPIConnection() :
        transmission{},
        reception{},
        packet_size{SPI_PACKET_SIZE} {
        transmission.resize(packet_size);
        reception.resize(packet_size);
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
        if (reception[0] == 1 &&
            reception[1] == 2 &&
            reception[2] == 4 &&
            reception[3] == 8 &&
            reception[4] == 7 &&
            reception[5] == 5) {
            uint8_t header_index = reception[6];
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
        spi_write_read_blocking(SPI_DEVICE, transmission.data(), reception.data(), packet_size);
    }
};
}
