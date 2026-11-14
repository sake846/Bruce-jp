#pragma once

#include "standalone_display_font.h"

/* Shared scanline overlay, using each display's row buffer. */
static const char *const m5stack_menu_names[] = {"", "FREQ STEP", "FREQ SPAN"};

static void m5stack_menu_value(char *value, size_t size) {
    switch (controls.menu) {
    case M5_MENU_STEP: snprintf(value, size, "%u MHZ", m5stack_steps[controls.step]); break;
    case M5_MENU_SPAN: snprintf(value, size, "%u MHZ", m5stack_spans[controls.rate]); break;
    default: value[0] = '\0'; break;
    }
}

static void m5stack_menu_value_row(unsigned y, const char *text, unsigned margin) {
    const GFXfont *font = &FreeSansBold12pt7b;
    unsigned width = m5stack_text_width(text, font);
    if (width > SCREEN_WIDTH - 2 * margin - 4) {
        font = &FreeSansBold9pt7b;
        width = m5stack_text_width(text, font);
    }
    int left = ((int)SCREEN_WIDTH - (int)width) / 2;
    int baseline = SCREEN_HEIGHT / 2 + 19;
    m5stack_text_row(y, left, baseline, text, font, COLOR_SRDECK_YELLOW_GREEN, margin + 1);
}

static void m5stack_menu_row(unsigned y, const char *value) {
    unsigned top = SCREEN_HEIGHT / 2 - 34, bottom = SCREEN_HEIGHT / 2 + 34;
    if (y < top || y > bottom) return;
    unsigned margin = SCREEN_WIDTH > 200 ? 32 : 4;
    for (unsigned x = margin; x < SCREEN_WIDTH - margin; x++)
        row[x] = __builtin_bswap16(y == top || y == bottom || x == margin ||
                                   x == SCREEN_WIDTH - margin - 1 ? COLOR_SRDECK_YELLOW_GREEN : 0x0861);
    const char *name = m5stack_menu_names[controls.menu];
    text_row(y - (SCREEN_HEIGHT / 2 - 23), ((int)SCREEN_WIDTH - (int)text_width(name)) / 2, name, 0xffff);
    m5stack_menu_value_row(y, value, margin);
}
