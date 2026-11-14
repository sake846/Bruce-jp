#include "rf_waterfall.h"
#include <vector>
#ifndef TFT_MOSI
#define TFT_MOSI -1
#endif
float m_rf_waterfall_start_freq = 433.0;
float m_rf_waterfall_end_freq = 435.0;

void rf_waterfall() {
    if (bruceConfigPins.rfModule != CC1101_SPI_MODULE) {
        displayError("Waterfall needs a CC1101!", true);
        return;
    }
    if (!initRfModule("rx", m_rf_waterfall_start_freq)) {
        displayError("CC1101 not found!", true);
        return;
    }

    ELECHOUSE_cc1101.setRxBW(200);
    int option, idx = 0;
select:

    option = 0;
    options = {
        {"Start Waterfall", [&]() { option = 3; }},
        {"Start Freq.",     [&]() { option = 1; }},
        {"End Freq.",       [&]() { option = 2; }},
        {"Main Menu",       [&]() { option = 4; }},
    };
    idx = loopOptions(options, idx);

    tft.fillScreen(0x0);

    if (option == 4) {
        return;
    } else if (option == 3) {
        rf_waterfall_run();
        goto select;
    } else if (option == 1) {
        rf_waterfall_boundary_freq(m_rf_waterfall_start_freq);
        goto select;
    } else if (option == 2) {
        rf_waterfall_boundary_freq(m_rf_waterfall_end_freq);
        goto select;
    }
}
void rf_waterfall_boundary_freq(float &boundary) {
    options = {};
    int ind = 0;
    int arraySize = sizeof(subghz_frequency_list) / sizeof(subghz_frequency_list[0]);
    for (int i = 0; i < arraySize; i++) {
        if (subghz_frequency_list[i] - boundary < 0.1) ind = i;
        String tmp = String(subghz_frequency_list[i], 2) + "Mhz";
        options.push_back({tmp.c_str(), [&boundary, i]() { boundary = subghz_frequency_list[i]; }});
    }
    loopOptions(options, ind);
    options.clear();
}

uint16_t swapBytes(uint16_t c) { return (c >> 8) | (c << 8); }

void rf_waterfall_run() {
    float f_start = m_rf_waterfall_start_freq;
    float f_end = m_rf_waterfall_end_freq;
    const int screen_width = tft.width();
    const int screen_height = tft.height();
    const int display_top = screen_height / 5;
    const int plot_left = 34;
    const int plot_width = screen_width - plot_left;
    const int plot_height = screen_height - display_top;
    float f_freq_step;

    // Alloc framebuffer
    uint16_t frameBuffer[screen_width] = {0};
    std::vector<unsigned long> row_scan_numbers(plot_height, 0);

    int current_line = display_top;
    unsigned long scan_number = 0;
    initRfModule("rx", f_start);

    float max_freq = f_start;
    int max_rssi = -100;
    unsigned long lastMaxUpdate = millis();
    int selected_item = 0;
    bool exitting = false;
    unsigned long exit_time = 0;

    tft.fillScreen(TFT_BLACK);

    auto draw_frequency_grid = [&]() {
        tft.setTextSize(1);
        for (int tick = 0; tick <= 4; ++tick) {
            const int x = plot_left + (plot_width - 1) * tick / 4;
            const float frequency = f_start + (f_end - f_start) * tick / 4.0f;
            char label[12];
            snprintf(label, sizeof(label), "%.1f", frequency);
            tft.setTextColor(
                (tick == 0 && selected_item == 0) || (tick == 4 && selected_item == 1) ? TFT_PINK
                                                                                        : TFT_WHITE,
                TFT_BLACK
            );
            if (tick == 0) tft.drawString(label, x, 0);
            else if (tick == 4) tft.drawRightString(label, x, 0);
            else tft.drawCentreString(label, x, 0);
            tft.drawFastVLine(x, display_top, plot_height, TFT_DARKGREY);
        }
    };
    auto draw_scan_age_grid = [&]() {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        for (int tick = 0; tick <= 4; ++tick) {
            const int y = display_top + (plot_height - 1) * tick / 4;
            const unsigned long row_scan = row_scan_numbers[y - display_top];
            const String label = row_scan == 0 ? "--" : String(scan_number - row_scan);
            const int label_y = min(y - 4, screen_height - 8);
            tft.fillRect(0, label_y, plot_left - 3, 8, TFT_BLACK);
            tft.drawRightString(label, plot_left - 4, label_y);
            tft.drawFastHLine(plot_left, y, plot_width, TFT_DARKGREY);
        }
    };
    tft.setCursor(0, 10);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.print("sweeps ago");
    tft.drawCentreString("Frequency (MHz)", plot_left + plot_width / 2, 10);
    tft.setCursor(plot_left, 20);
    tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
    tft.print("[OK] Item [PREV/NEXT] Value ");
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.print("EXIT");
    draw_frequency_grid();
    draw_scan_age_grid();

    while (1) {
        draw_frequency_grid();

        f_freq_step = (f_end - f_start) / (plot_width - 1);

        float temp_max_freq = f_start;
        int temp_max_rssi = -100;

        float range = abs(f_end - f_start);
        float step;

        if (range > 100) step = 10;
        else if (range > 10) step = 1;
        else if (range > 1) step = 0.1;
        else if (range > 0.1) step = 0.01;
        else step = 0.001;

        for (int i = 0; i < plot_width; ++i) {
            float f_freq = f_start + i * f_freq_step;
            setMHZ(f_freq);
            // To make sure CC1101 shared with TFT works properly on T-Embed
            if (bruceConfigPins.CC1101_bus.mosi == TFT_MOSI) {
                tft.drawPixel(0, 0, 0);
                delayMicroseconds(150); // T-Embed case, need more time to process
            } else delayMicroseconds(100);

            int i_rssi = ELECHOUSE_cc1101.getRssi();
            // To make sure CC1101 shared with TFT works properly on T-Embed
            if (bruceConfigPins.CC1101_bus.mosi == TFT_MOSI) tft.drawPixel(0, 0, 0);
            if (i_rssi > temp_max_rssi) {
                temp_max_rssi = i_rssi;
                temp_max_freq = f_freq;
            }

            int rawLevel = map(i_rssi, -100, -30, 0, 255);
            int level = 255 - constrain(rawLevel, 0, 255);

            uint8_t r = 0, g = 0, b = 0;
            if (level <= 63) {
                b = map(level, 0, 63, 64, 255);
            } else if (level <= 127) {
                g = map(level, 64, 127, 0, 255);
                b = map(level, 64, 127, 255, 0);
            } else if (level <= 191) {
                r = map(level, 128, 191, 0, 255);
                g = 255;
            } else {
                r = 255;
                g = map(level, 192, 255, 255, 0);
            }

            uint16_t color = tft.color565(r, g, b);
            frameBuffer[i] = swapBytes(color);
            if (check(SelPress)) {
                selected_item++;
                if (selected_item > 2) selected_item = 0;
            }

            if (check(UpPress) || check(NextPress)) {
                switch (selected_item) {
                    case 0: f_start += step; break;
                    case 1: f_end += step; break;
                    case 2: return;
                }
                delay(100);
            } else if (check(DownPress) || check(PrevPress)) {
                switch (selected_item) {
                    case 0: f_start -= step; break;
                    case 1: f_end -= step; break;
                    case 2: return;
                }
                if (EscPress) EscPress = false; // Reset for StickCs
                delay(100);
            }
        }
        tft.drawPixel(0, 0, 0); // Cardputer Case, need to call something to the tft.
        tft.pushImage(plot_left, current_line, plot_width, 1, frameBuffer);
        row_scan_numbers[current_line - display_top] = ++scan_number;
        tft.drawFastHLine(plot_left, current_line, plot_width, TFT_DARKGREY);
        for (int tick = 0; tick <= 4; ++tick) {
            const int x = plot_left + (plot_width - 1) * tick / 4;
            tft.drawPixel(x, current_line, TFT_DARKGREY);
        }
        draw_scan_age_grid();

        if (millis() - lastMaxUpdate >= 5000) {
            max_rssi = temp_max_rssi;
            max_freq = temp_max_freq;
            tft.fillRect(0, 10, screen_width, 10, TFT_BLACK);
            tft.setCursor(3, 10);
            tft.setTextSize(1);
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);
            tft.printf("%d dBm @ %.3f", max_rssi, max_freq);

            lastMaxUpdate = millis();
        }

        tft.fillRect(plot_left, 20, screen_width - plot_left, 10, TFT_BLACK);
        tft.setCursor(plot_left, 20);
        tft.setTextColor(TFT_DARKCYAN);
        tft.print("[OK] Item [PREV/NEXT] Value ");

        if (selected_item == 2) {
            tft.setTextColor(TFT_RED);
            tft.print("EXIT");
        } else {
            tft.setTextColor(TFT_WHITE);
            tft.print("EXIT");
        }

        if (check(EscPress)) break;

        current_line++;
        if (current_line >= screen_height) current_line = display_top;
    }

    returnToMenu = true;
    deinitRfModule();
    delay(10);
}
