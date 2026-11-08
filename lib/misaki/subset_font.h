#pragma once
// Free Japanese & Latin Font Subset with Per-Size On-Demand Storage Optimization
// Size 1 (8px):   Misaki Font (CJK) / M+ (ASCII) [All glyphs]
// Size 2 (16px):  Shinonome Font (CJK) / M+ (ASCII) [Menus, UI, Kana]
// Size 3 (24px):  M+ Font (SIL OFL 1.1) [Selected menus, ASCII]
// Size 4 (32px):  M+ Font (SIL OFL 1.1) [Clock, Timer, ASCII]

#include <stdint.h>
#include <stddef.h>

struct SubsetGlyph {
    uint16_t unicode;
    uint8_t width;          // 8 for fullwidth, 6 for ASCII, 4 for halfwidth kana
    uint8_t data8[8];       // 8x8 font bitmap (size 1) - always present
    const uint8_t* data16;  // 16x16 font bitmap (size 2: 32 bytes) or nullptr
    const uint8_t* data24;  // 24x24 font bitmap (size 3: 72 bytes) or nullptr
    const uint8_t* data32;  // 32x32 font bitmap (size 4: 128 bytes) or nullptr
};

extern const SubsetGlyph kSubsetGlyphs[];
extern const size_t kSubsetGlyphsCount;

const SubsetGlyph* getSubsetGlyph(uint16_t unicode);
