# pragma once
#include "paradigm.h"

#include <stdio.h>
#include <hardware/dma.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// 50 khz sample frequency
#define ADC_CLOCK_DIV 960

inline int dma_channel;
inline dma_channel_config dma_config;
inline uint8_t dma_capture_buffer[FFT_FRAME_SIZE];

namespace quetzal {
class Sigin : public Name {
    int capture_channel;
    int gpio_pin;

public:
    Sigin(int capture_channel) :
        capture_channel{capture_channel} {
        int gpio_pin = 26 + capture_channel;
        init_adc();
        init_dma();
    }

    void init_adc() {
        stdio_init_all();
        adc_init();
        adc_gpio_init(gpio_pin);
        adc_select_input(capture_channel);
        adc_set_clkdiv(ADC_CLOCK_DIV);
        adc_fifo_setup(
            true,
            true,
            1,
            false,
            true
        );
    }

    void init_dma() {
        dma_channel = dma_claim_unused_channel(true);
        dma_config = dma_channel_get_default_config(dma_channel);
        channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
        channel_config_set_read_increment(&dma_config, false);
        channel_config_set_write_increment(&dma_config, true);
        channel_config_set_dreq(&dma_config, DREQ_ADC);
        channel_config_set_high_priority(&dma_config, true);
    }

    uptr<Signal<float>> collect_frame() {
        dma_channel_configure(dma_channel,
                              &dma_config,
                              dma_capture_buffer,
                              &adc_hw->fifo,
                              FFT_FRAME_SIZE,
                              true
        );
        adc_run(true);
        dma_channel_wait_for_finish_blocking(dma_channel);
        adc_run(false);
        adc_fifo_drain();

        auto signal = mkuptr<Signal<float>>();
        for (int i = 0; i < FFT_FRAME_SIZE; i++) {
            signal->push_back(dma_capture_buffer[i]);
        }
        return signal;
    }
};
}
