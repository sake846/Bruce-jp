#include "rf_spectrum.h"
#include "protocols/rf_config.h"
#include "protocols/rf_decoder.h"
#include "rf_utils.h"
#include "structs.h"

static bool spectrum_rmt_rx_done_callback(
    rmt_channel_t *channel, const rmt_rx_done_event_data_t *edata, void *user_data
) {
    BaseType_t high_task_wakeup = pdFALSE;
    QueueHandle_t receive_queue = (QueueHandle_t)user_data;
    // send the received RMT symbols to the parser task
    xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
}

static constexpr int RF_PLOT_LEFT = 42;
static constexpr int RF_PLOT_TOP = 24;
static constexpr int RF_PLOT_BOTTOM_MARGIN = 14;

void draw_tf_spectrum_grid() {
    const int plot_right = tftWidth - 1;
    const int plot_bottom = tftHeight - RF_PLOT_BOTTOM_MARGIN;
    const int plot_height = plot_bottom - RF_PLOT_TOP;
    tft.setTextSize(1);
    tft.setCursor(3, 2);
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.printf(" RF - Spectrum (%.2f Mhz)", bruceConfigPins.rfFreq);
    tft.fillRect(0, 20, tftWidth, tftHeight - 20, bruceConfig.bgColor);
    for (int tick = 0; tick <= 4; ++tick) {
        const int x = RF_PLOT_LEFT + (plot_right - RF_PLOT_LEFT) * tick / 4;
        tft.drawFastVLine(x, RF_PLOT_TOP, plot_bottom - RF_PLOT_TOP, TFT_DARKGREY);
        const String label = String(tick * 25) + "%";
        if (tick == 0) tft.drawString(label, RF_PLOT_LEFT, plot_bottom + 2);
        else if (tick == 4) tft.drawRightString(label, plot_right, plot_bottom + 2);
        else tft.drawCentreString(label, x, plot_bottom + 2);
    }
    for (int duration = 0; duration <= SIGNAL_STRENGTH_THRESHOLD; duration += 500) {
        const int y = plot_bottom - map(duration, 0, SIGNAL_STRENGTH_THRESHOLD, 0, plot_height);
        tft.drawFastHLine(RF_PLOT_LEFT, y, plot_right - RF_PLOT_LEFT + 1, TFT_DARKGREY);
        tft.drawString(String(duration) + "us", 0, y - 3);
    }
}

void rf_spectrum() {
    tft.fillScreen(bruceConfig.bgColor);
    draw_tf_spectrum_grid();
    const int plot_right = tftWidth - 1;
    const int plot_bottom = tftHeight - RF_PLOT_BOTTOM_MARGIN;
    const int plot_height = plot_bottom - RF_PLOT_TOP;
    if (bruceConfigPins.rfModule == M5_RF_MODULE) {
        RfRxSession rx;
        if (!rx.begin()) {
            deinitRfModule();
            return;
        }

        std::vector<int> durations;
        while (1) {
            if (rx.poll(durations)) {
                draw_tf_spectrum_grid();
                for (size_t i = 0; i < durations.size(); i++) {
                    const int pulse = constrain(abs(durations[i]), 0, SIGNAL_STRENGTH_THRESHOLD);
                    const int lineHeight = map(pulse, 0, SIGNAL_STRENGTH_THRESHOLD, 0, plot_height);
                    int lineX =
                        map(i, 0, durations.size() > 1 ? durations.size() - 1 : 1, RF_PLOT_LEFT, plot_right);
                    tft.drawFastVLine(lineX, plot_bottom - lineHeight, lineHeight, bruceConfig.priColor);
                }
                RF_DBG("m5 spectrum: durations=%u", (unsigned)durations.size());
            }

            if (check(EscPress)) { break; }
            if (setMHZMenu()) {
                rx.end();
                rx.begin();
                draw_tf_spectrum_grid();
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        rx.end();
        returnToMenu = true;
        deinitRfModule();
        return;
    }

    rmt_channel_handle_t rx_ch = NULL;
    rx_ch = setup_rf_rx();
    if (rx_ch == NULL) return;
    ESP_LOGI("RMT_SPECTRUM", "register RX done callback");
    QueueHandle_t receive_queue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
    assert(receive_queue);
    rmt_rx_event_callbacks_t cbs = {
        .on_recv_done = spectrum_rmt_rx_done_callback,
    };
    ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rx_ch, &cbs, receive_queue));
    ESP_ERROR_CHECK(rmt_enable(rx_ch));
    rmt_receive_config_t receive_config = {
        .signal_range_min_ns = 3000,     // 6us minimum signal duration
        .signal_range_max_ns = 12000000, // 24ms maximum signal duration
    };
    rmt_symbol_word_t item[64];
    rmt_rx_done_event_data_t rx_data;
    ESP_ERROR_CHECK(rmt_receive(rx_ch, item, sizeof(item), &receive_config));

    size_t rx_size = 0;
    while (1) {
        rmt_symbol_word_t *rx_items = NULL;
        if (xQueueReceive(receive_queue, &rx_data, 0) == pdPASS) {
            rx_size = rx_data.num_symbols;
            rx_items = rx_data.received_symbols;
        }
        if (rx_size != 0) {
            // Draw grid and info
            draw_tf_spectrum_grid();
            // Draw waveform based on signal strength
            for (size_t i = 0; i < rx_size; i++) {
                int lineHeight =
                    map(rx_items[i].duration0 + rx_items[i].duration1,
                        0,
                        SIGNAL_STRENGTH_THRESHOLD,
                        0,
                        plot_height);
                lineHeight = constrain(lineHeight, 0, plot_height);
                int lineX = map(i, 0, rx_size > 1 ? rx_size - 1 : 1, RF_PLOT_LEFT, plot_right);
                tft.drawFastVLine(lineX, plot_bottom - lineHeight, lineHeight, bruceConfig.priColor);
            }

            ESP_ERROR_CHECK(rmt_receive(rx_ch, item, sizeof(item), &receive_config));
            rx_size = 0;
        }
        // Checks to leave while
        if (check(EscPress)) { break; }
        if (setMHZMenu()) yield();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    returnToMenu = true;
    rmt_disable(rx_ch);
    rmt_del_channel(rx_ch);
    vQueueDelete(receive_queue);
    deinitRfModule();
}

#define TIME_DIVIDER (tftWidth / 10)
//@Pirata
static constexpr int SQUARE_PLOT_LEFT = 32;
static constexpr int SQUARE_PLOT_TOP = 24;
static constexpr int SQUARE_PLOT_BOTTOM_MARGIN = 18;

static void draw_rf_squarewave_grid() {
    const int plot_right = tftWidth - 1;
    const int plot_bottom = tftHeight - SQUARE_PLOT_BOTTOM_MARGIN;
    const int plot_width = plot_right - SQUARE_PLOT_LEFT;
    tft.fillScreen(bruceConfig.bgColor);
    tft.setTextSize(1);
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.setCursor(3, 2);
    tft.printf("  RF - SquareWave (%.2f Mhz)", bruceConfigPins.rfFreq);

    for (int tick = 0; tick <= 4; ++tick) {
        const int x = SQUARE_PLOT_LEFT + plot_width * tick / 4;
        tft.drawFastVLine(x, SQUARE_PLOT_TOP, plot_bottom - SQUARE_PLOT_TOP, TFT_DARKGREY);
        char label[12];
        snprintf(label, sizeof(label), "%.1fms", plot_width * tick / 4.0f * TIME_DIVIDER / 1000.0f);
        if (tick == 0) tft.drawString(label, x, plot_bottom + 2);
        else if (tick == 4) tft.drawRightString(label, x, plot_bottom + 2);
        else tft.drawCentreString(label, x, plot_bottom + 2);
    }
    for (int y = SQUARE_PLOT_TOP; y + 6 <= plot_bottom; y += 10) {
        tft.drawFastHLine(SQUARE_PLOT_LEFT, y, plot_width, TFT_DARKGREY);
        tft.drawFastHLine(SQUARE_PLOT_LEFT, y + 6, plot_width, TFT_DARKGREY);
    }
    tft.drawString("1", 4, SQUARE_PLOT_TOP - 2);
    tft.drawString("0", 4, SQUARE_PLOT_TOP + 4);
}

void rf_SquareWave() {
    if (!initRfModule("rx", bruceConfigPins.rfFreq)) return;

    RfRxSession rx;
    if (!rx.begin()) {
        deinitRfModule();
        return;
    }
    int line_w = SQUARE_PLOT_LEFT;
    int line_h = SQUARE_PLOT_TOP;
    std::vector<int> durations;
PRINT:
    tft.drawPixel(0, 0, 0);
    draw_rf_squarewave_grid();
    line_w = SQUARE_PLOT_LEFT;
    line_h = SQUARE_PLOT_TOP;

    while (1) {
        if (rx.poll(durations)) {
            // Draw the captured square wave (HIGH width then LOW width per pair).
            for (size_t i = 0; i + 1 < durations.size(); i += 2) {
                int high = abs(durations[i]);
                int low = abs(durations[i + 1]);
                if (high == 0) break;

                if (high > 20000) high = 20000;
                if (low > 20000) low = 20000;
                if (line_w + (high + low) / TIME_DIVIDER > tftWidth) {
                    line_w = SQUARE_PLOT_LEFT;
                    line_h += 10;
                }
                if (line_h + 12 > tftHeight - SQUARE_PLOT_BOTTOM_MARGIN) {
                    draw_rf_squarewave_grid();
                    line_w = SQUARE_PLOT_LEFT;
                    line_h = SQUARE_PLOT_TOP;
                }
                tft.drawFastVLine(line_w, line_h, 6, bruceConfig.priColor);
                tft.drawFastHLine(line_w, line_h, high / TIME_DIVIDER, bruceConfig.priColor);

                tft.drawFastVLine(line_w + high / TIME_DIVIDER, line_h, 6, bruceConfig.priColor);
                tft.drawFastHLine(
                    line_w + high / TIME_DIVIDER, line_h + 6, low / TIME_DIVIDER, bruceConfig.priColor
                );
                line_w += (high + low) / TIME_DIVIDER;
            }
        }
        // Checks to leave while
        if (check(EscPress)) { break; }
        if (setMHZMenu()) goto PRINT;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    rx.end();
    returnToMenu = true;
}

void rf_CC1101_rssi() {
#if !defined(LITE_VERSION)
    if (bruceConfigPins.rfModule != CC1101_SPI_MODULE) {
        displayError("only for CC1101 module", true);
        return;
    }
    const int graph_left = 32;
    const int graph_size = tftWidth - graph_left;
    std::vector<int> signal(graph_size, -95);
    const size_t freq_count = sizeof(subghz_frequency_list) / sizeof(float);
    std::vector<int> bar_size(freq_count, 0);
    int max_bar_size = tftHeight - 20 /*bottom margin*/ - 20 /*top margin*/;
    bool redraw = true;
    const int min_value = map(-70, -95, -20, 0, max_bar_size);
    auto draw_fixed_grid = [&]() {
        const int base_y = tftHeight - 120;
        const int plot_top = base_y + 20;
        const int plot_bottom = base_y + 95;
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
        for (int value = -95; value <= -20; value += 15) {
            const int y = base_y - value;
            tft.drawFastHLine(graph_left, y, tftWidth - graph_left, TFT_DARKGREY);
            tft.drawString(String(value), 0, y - 3);
        }
        for (int tick = 0; tick <= 4; ++tick) {
            const int x = graph_left + (graph_size - 1) * tick / 4;
            tft.drawFastVLine(x, plot_top, plot_bottom - plot_top, TFT_DARKGREY);
            const int seconds_ago = (graph_size - 1) * (4 - tick) * 75 / 4000;
            const String label = tick == 4 ? "0s" : "-" + String(seconds_ago) + "s";
            if (tick == 0) tft.drawString(label, x, tftHeight - 10);
            else if (tick == 4) tft.drawRightString(label, x, tftHeight - 10);
            else tft.drawCentreString(label, x, tftHeight - 10);
        }
    };
    auto draw_range_grid = [&](int range) {
        const int plot_top = 20;
        const int plot_bottom = tftHeight - 20;
        const int plot_left = graph_left;
        const int plot_width = tftWidth - plot_left;
        const int first_freq = range_limits[bruceConfigPins.rfScanRange][0];
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
        for (int value = -95; value <= -20; value += 15) {
            const int y = plot_bottom - map(value, -95, -20, 0, max_bar_size);
            tft.drawFastHLine(plot_left, y, plot_width, TFT_DARKGREY);
            tft.drawString(String(value), 0, y - 3);
        }
        for (int tick = 0; tick <= 4; ++tick) {
            const int point = (range - 1) * tick / 4;
            const int x = plot_left + (plot_width - 1) * tick / 4;
            const float frequency = subghz_frequency_list[first_freq + point];
            char label[12];
            snprintf(label, sizeof(label), "%.3f", frequency);
            tft.drawFastVLine(x, plot_top, plot_bottom - plot_top, TFT_DARKGREY);
            if (tick == 0) tft.drawString(label, x, tftHeight - 10);
            else if (tick == 4) tft.drawRightString(label, x, tftHeight - 10);
            else tft.drawCentreString(label, x, tftHeight - 10);
        }
    };
    while (1) {
        if (redraw) {
            redraw = false;
            tft.drawPixel(0, 0, 0);
            tft.fillScreen(bruceConfig.bgColor);
            tft.setTextSize(1);
            tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
            tft.setCursor(3, 2);
            // Fixed frequency sees a dot running grafic, showing RSSI over time
            if (bruceConfigPins.rfFxdFreq) {
                if (!initRfModule("rx", bruceConfigPins.rfFreq))
                    displayError("Error setting frequency", true);
                tft.printf(" RF - RSSI (dBm) spectrum (%.2f Mhz)", bruceConfigPins.rfFreq);
                draw_fixed_grid();
                // resets signal array
                std::fill(signal.begin(), signal.end(), -95);
            }
            // Range Scan Sees a bargraph simillar to NRF24 grafic, using RSSI across frequencies
            else {
                if (!initRfModule("rx", bruceConfigPins.rfFreq)) displayError("Error starting module", true);
                tft.printf(
                    " RF - RSSI (dBm) spectrum (%s)", subghz_frequency_ranges[bruceConfigPins.rfScanRange]
                );
                int range = range_limits[bruceConfigPins.rfScanRange][1] -
                            range_limits[bruceConfigPins.rfScanRange][0] + 1;
                draw_range_grid(range);
                std::fill(bar_size.begin(), bar_size.end(), 0);
            }
        }

        // draw dot graph for fixed frequency
        if (bruceConfigPins.rfFxdFreq) {
            int rssi = ELECHOUSE_cc1101.getRssi();
            tft.drawPixel(0, 0, 0); // To make sure CC1101 shared with TFT works properly
            const int base_y = tftHeight - 120;
            int prev = signal[0];
            for (int i = 1; i < graph_size; i++) {
                if (EscPress || SelPress) break;
                const int x0 = graph_left + (i - 1);
                const int x1 = graph_left + i;
                const int curr = signal[i];
                // erase old segment between previous and current points
                tft.drawLine(x0, base_y - prev, x1, base_y - curr, bruceConfig.bgColor);
                const int next_val = (i == graph_size - 1) ? rssi : signal[i + 1];
                // shift buffer left by one
                signal[i - 1] = curr;
                if (i == graph_size - 1) signal[i] = rssi;
                // draw updated segment using new values
                tft.drawLine(x0, base_y - curr, x1, base_y - next_val, bruceConfig.priColor);
                prev = curr;
            }
            draw_fixed_grid();
            vTaskDelay(pdMS_TO_TICKS(75));
        }
        // draw a bargraph similar to nrf24 across the range
        else {
            int range = range_limits[bruceConfigPins.rfScanRange][1] -
                        range_limits[bruceConfigPins.rfScanRange][0] + 1;

            const int plot_width = tftWidth - graph_left;
            int space = plot_width / range;
            int max_idx = 0;
            for (int i = 0; i < range; i++) {
                if (EscPress || SelPress) break;
                setMHZ(subghz_frequency_list[range_limits[bruceConfigPins.rfScanRange][0] + i]);
                vTaskDelay(pdMS_TO_TICKS(5));
                int rssi = ELECHOUSE_cc1101.getRssi();
                tft.drawPixel(0, 0, 0); // To make sure CC1101 shared with TFT works properly
                int size = map(rssi, -95, -20, 0, max_bar_size);
                if (size > bar_size[i]) bar_size[i] = size;
                else bar_size[i] = bar_size[i] - (bar_size[i] - size) / 2; // slow down decrease
                tft.fillRect(
                    graph_left + i * space, tftHeight - 20 - bar_size[i], space - 2, bar_size[i], bruceConfig.priColor
                );
                tft.fillRect(
                    graph_left + i * space, 20, space, max_bar_size - bar_size[i], bruceConfig.bgColor
                );
                if (bar_size[i] > bar_size[max_idx] && bar_size[i] > min_value) max_idx = i;
            }
            draw_range_grid(range);
            if (bar_size[max_idx] > min_value) {
                char buf[7];
                float var = subghz_frequency_list[range_limits[bruceConfigPins.rfScanRange][0] + max_idx];
                snprintf(buf, sizeof(buf), "%.2f", var);
                tft.drawCentreString("Max=      ", tftWidth / 2, 10);
                tft.drawCentreString("Max=" + String(buf), tftWidth / 2, 10);
            }
        }
        if (check(EscPress)) { break; }
        if (check(SelPress)) {
            deinitRfModule();
            rf_range_selection(bruceConfigPins.rfFreq);
            redraw = true;
        }
    }
    deinitRfModule();
#else
    displayError("Not available on Launcher version");
#endif
}
