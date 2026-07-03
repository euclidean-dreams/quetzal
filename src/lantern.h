#pragma once

#include "paradigm.h"
#include "pico/stdlib.h"
#include "generated/ws2812.pio.h"

#define WS2812_HAS_W false
#define WS2812_PIN 15

namespace quetzal {
class Lantern : public Name {
public:
    uptr<Lattice> lattice;
    int led_count;

    Lantern(int width, int height)
        : led_count{width * height} {
        lattice = mkuptr<Lattice>(width, height, Pith{{0, 0, 0}}, false);
        auto pio = pio0;
        int sm = 0;
        uint offset = pio_add_program(pio, &ws2812_program);
        ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, WS2812_HAS_W);
        for (int i = 0; i < led_count; i++) {
            put_pixel(serialize_color(Color{0, 0, 0}));
        }
        sleep_ms(10);
    }

    void show_test_pattern() {
        for (int i = 0; i < led_count; i += led_count / 10) {
            auto size = 3;
            auto start_index = i - size / 2;
            auto end_index = i + size / 2;

            for (int j = 0; j < start_index; j++) {
                put_pixel(serialize_color(Color{0, 0, 0}));
            }
            for (int j = start_index; j < end_index && j < led_count; j++) {
                put_pixel(serialize_color(Color{0, 33, 77}));
            }
            for (int j = end_index; j < led_count; j++) {
                put_pixel(serialize_color(Color{0, 0, 0}));
            }
            sleep_ms(10);
        }
        for (int i = led_count; i > 0; i -= led_count / 10) {
            auto size = 3;
            auto start_index = i - size / 2;
            auto end_index = i + size / 2;

            for (int j = 0; j < start_index; j++) {
                put_pixel(serialize_color(Color{0, 0, 0}));
            }
            for (int j = start_index; j < end_index && j < led_count; j++) {
                put_pixel(serialize_color(Color{0, 33, 77}));
            }
            for (int j = end_index; j < led_count; j++) {
                put_pixel(serialize_color(Color{0, 0, 0}));
            }
            sleep_ms(10);
        }
    }

    static uint32_t serialize_color(Color color) {
        return (scast<uint32_t>(color.green) << 16) | (scast<uint32_t>(color.red) << 8) | scast<uint32_t>(color.blue);
    }

    static inline void put_pixel(uint32_t pixel_grb) {
        pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
    }

    void switch_lattice(uptr<Lattice> fresh_lattice) {
        lattice = mv(fresh_lattice);
    }

    void show_switchback() {
        for (int x = lattice->width - 1; x >= 0; x--) {
            if (x % 2 != 0) {
                for (int y = 0; y < lattice->height; y++) {
                    auto pith = lattice->get_pith(x, y);
                    auto serialized_color = serialize_color(pith.color);
                    put_pixel(serialized_color);
                }
            } else {
                for (int y = lattice->height - 1; y >= 0; y--) {
                    auto pith = lattice->get_pith(x, y);
                    auto serialized_color = serialize_color(pith.color);
                    put_pixel(serialized_color);
                }
            }
        }
    }

    void show_toroidalack() {
        for (int y = 0; y < RENDER_HEIGHT; y++) {
            for (int x = 0; x < RENDER_WIDTH; x++) {
                auto pith = lattice->get_pith(x, y);
                auto serialized_color = serialize_color(pith.color);
                put_pixel(serialized_color);
            }
        }
    }
};
}
