#pragma once
// Free Japanese & Latin Font Subset with Direct Display Size Optimization
// Size 1 (8px):   Misaki Font (CJK) / M+ (ASCII)
// Size 2 (16px):  Shinonome Font (CJK) / M+ (ASCII)
// Size 3 (24px):  M+ Font (SIL OFL 1.1) for all glyphs

#include <stdint.h>
#include <stddef.h>

struct SubsetGlyph {
    uint16_t unicode;
    uint8_t width;      // 8 for fullwidth, 6 for ASCII, 4 for halfwidth kana
    uint8_t data8[8];   // 8x8 font bitmap (size 1: 1 byte per row)
    uint8_t data16[32]; // 16x16 font bitmap (size 2: 2 bytes per row, MSB on left)
    uint8_t data24[72]; // 24x24 font bitmap (size 3: 3 bytes per row, MSB on left)
};

extern const SubsetGlyph kSubsetGlyphs[];
extern const size_t kSubsetGlyphsCount;

const SubsetGlyph* getSubsetGlyph(uint16_t unicode);
