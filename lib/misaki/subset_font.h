#pragma once
// Free Japanese Font Subset
// 16x16: Shinonome Font (Public Domain) / M+ Font (SIL OFL 1.1)
// 8x8:   Misaki Font (Free / Public Domain)

#include <stdint.h>
#include <stddef.h>

struct SubsetGlyph {
    uint16_t unicode;
    uint8_t width;      // 8 for fullwidth, 4 for halfwidth
    uint8_t data8[8];   // 8x8 font bitmap (1 byte per row)
    uint8_t data16[32]; // 16x16 font bitmap (2 bytes per row, MSB on left)
};

extern const SubsetGlyph kSubsetGlyphs[];
extern const size_t kSubsetGlyphsCount;

const SubsetGlyph* getSubsetGlyph(uint16_t unicode);
