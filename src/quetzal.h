#pragma once

#include "paradigm.h"
#include "spi_connection.h"
#include <iostream>
#include "lantern.h"
#include "DmxOutput.h"

DmxOutput dmx;
#define DMX_FRAME_LENGTH 512
uint8_t universe[DMX_FRAME_LENGTH + 1];

namespace quetzal {
class Quetzal : public Name {
public:
    uptr<SPIConnection> spi_connection;
    uptr<Lantern> lantern;
    uptr<Cosmology> cosmology;

    Quetzal() {
        cosmology = mkuptr<Cosmology>(RENDER_WIDTH, RENDER_HEIGHT, 0, Impressions::workshop);
        spi_connection = mkuptr<SPIConnection>();
        std::cout << "spi ready" << std::endl;

        lantern = mkuptr<Lantern>(OBSERVATION_WIDTH, OBSERVATION_HEIGHT);
        std::cout << "ws2812 lantern ready" << std::endl;

        lantern->show_test_pattern();
        std::cout << "ws2812 lantern showing test pattern" << std::endl;
    }

    void keyhole_loop() {
        while (true) {
            spi_connection->exchange();
            if (spi_connection->spi_header_is_valid()) {
                auto x = 0;
                auto y = 0;
                for (int i = HEADER_SIZE; i < SPI_PACKET_SIZE; i += 3) {
                    auto color = Color{
                        spi_connection->receive_buffer[i],
                        spi_connection->receive_buffer[i + 1],
                        spi_connection->receive_buffer[i + 2]
                    };
                    lantern->lattice.set_pith(x, y, Pith{color});
                    x++;
                    if (x % RENDER_WIDTH == 0) {
                        x = 0;
                        y++;
                    }
                }
                lantern->show();
            } else {
                std::cout << "encountered invalid spi header, re-initializing spi..." << std::endl;
                // the spi hardware will happily begin reading halfway through a transmission, as well as other nonsense
                // if we encounter a transmission without a valid header, drop it and reset the SPI

                spi_connection = mkuptr<SPIConnection>();
            }
        }
    }

    void dmx_loop() {
        dmx.begin(0);
        for (int i = 1; i < DMX_FRAME_LENGTH + 1; i++) {
            universe[i] = 0;
        }

        while (true) {
            spi_connection->exchange();
            if (spi_connection->spi_header_is_valid()) {
                for (int i = 1; i < DMX_FRAME_LENGTH + 1; i++) {
                    universe[i] = spi_connection->receive_buffer[i - 1 + HEADER_SIZE];
                }
                while (dmx.busy()) {
                    /* Do nothing while the DMX frame transmits */
                }
                dmx.write(universe, DMX_FRAME_LENGTH);
            } else {
                std::cout << "encountered invalid spi header, re-initializing spi..." << std::endl;
                // the spi hardware will happily begin reading halfway through a transmission, as well as other nonsense
                // if we encounter a transmission without a valid header, drop it and reset the SPI
                spi_connection = mkuptr<SPIConnection>();
            }
        }
    }
};
}
