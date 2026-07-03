#pragma once

#include "paradigm.h"
#include "spi_connection.h"
#include <iostream>
#include "lantern.h"
#include "DmxOutput.h"
#include "sigin.h"
#include "perception/kiss_fft_transformer.h"
#include "perception/equalizer.h"

DmxOutput dmx;
#define DMX_FRAME_LENGTH 512
uint8_t universe[DMX_FRAME_LENGTH + 1];

namespace quetzal {
class Quetzal : public Name {
public:
    uptr<SPIConnection> spi_connection;
    uptr<Lantern> lantern;
    uptr<Cosmology> cosmology;
    uptr<Sigin> sigin;
    uptr<FourierTransform> fourier_transform;
    uptr<Equalizer> equalizer;

    Quetzal() {
        lantern = mkuptr<Lantern>(RENDER_WIDTH, RENDER_HEIGHT);
        std::cout << "lantern ready" << std::endl;

        lantern->show_test_pattern();
        std::cout << "lantern showing test pattern" << std::endl;
#ifdef SIGURD
        sigin = mkuptr<Sigin>(0);
        std::cout << "sigin ready" << std::endl;

        fourier_transform = mkuptr<FourierTransform>();
        std::cout << "fft ready" << std::endl;

        equalizer = mkuptr<Equalizer>(0.01);
        std::cout << "equalizer ready" << std::endl;

        cosmology = mkuptr<Cosmology>(RENDER_WIDTH, RENDER_HEIGHT, STFT_SIZE, Impressions::ambiance);
        std::cout << "cosmology ready" << std::endl;
#endif
#ifdef KEYHOLE
        spi_connection = mkuptr<SPIConnection>();
        std::cout << "spi ready" << std::endl;
#endif
#ifdef DMX
        spi_connection = mkuptr<SPIConnection>();
        std::cout << "spi ready" << std::endl;
#endif
    }

    void keyhole_loop() {
        while (true) {
            spi_connection->exchange();
            if (spi_connection->spi_header_is_valid()) {
                auto x = 0;
                auto y = 0;
                for (int i = HEADER_SIZE; i < spi_connection->packet_size; i += 3) {
                    auto color = Color{
                        spi_connection->reception[i],
                        spi_connection->reception[i + 1],
                        spi_connection->reception[i + 2]
                    };
                    lantern->lattice->set_pith(x, y, Pith{color});
                    x++;
                    if (x % RENDER_WIDTH == 0) {
                        x = 0;
                        y++;
                    }
                }
                lantern->show_toroidalack();
            } else {
                std::cout << "encountered invalid spi header, re-initializing spi..." << std::endl;
                // the spi hardware will happily begin reading halfway through a transmission, as well as other nonsense
                // if we encounter a transmission without a valid header, drop it and reset the SPI

                spi_connection = mkuptr<SPIConnection>();
            }
        }
    }

    void sigurd_loop() {
        while (true) {
            auto raw_audio_signal = sigin->collect_frame();
            auto equalized_signal = equalizer->equalize(mv(raw_audio_signal));
            auto stft = fourier_transform->stft(mv(equalized_signal));
            if (stft == nullptr) {
                continue;
            }

            auto stft_magnitudes = mksptr<Signal<float>>();
            for (auto &sample: *stft) {
                auto magnitude = scast<float>(std::sqrt(std::pow(sample.real(), 2) + std::pow(sample.imag(), 2)));
                stft_magnitudes->push_back(magnitude);
            }
            cosmology->experience(stft_magnitudes);
            auto lattice = cosmology->observe();

            lantern->switch_lattice(mv(lattice));
            lantern->show_toroidalack();
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
                    universe[i] = spi_connection->reception[i - 1 + HEADER_SIZE];
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
