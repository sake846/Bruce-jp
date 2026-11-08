import os
import re
from PIL import Image, ImageFont, ImageDraw

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
I18N_PATH = os.path.join(REPO_ROOT, "include", "i18n.h")
MISAKI_HPP = os.path.join(REPO_ROOT, "tools", "misaki_raw", "misaki.hpp")
SHINONOME_BIT = os.path.join(REPO_ROOT, "tools", "fonts", "shnmk16.bit")
MPLUS_TTF = os.path.join(REPO_ROOT, "tools", "fonts", "MPLUS1p-Medium.ttf")
OUT_HEADER = os.path.join(REPO_ROOT, "lib", "misaki", "subset_font.h")
OUT_CPP = os.path.join(REPO_ROOT, "lib", "misaki", "subset_font.cpp")

def collect_chars():
    chars = set()

    # 1. Full printable ASCII (0x20 to 0x7E)
    chars.update(chr(c) for c in range(0x20, 0x7F))

    # 2. i18n.h
    if os.path.exists(I18N_PATH):
        with open(I18N_PATH, "r", encoding="utf-8", errors="ignore") as f:
            for c in f.read():
                if 128 <= ord(c) <= 0xFFFF:
                    chars.add(c)

    # 3. Scan src for non-ascii
    src_dir = os.path.join(REPO_ROOT, "src")
    for root, _, files in os.walk(src_dir):
        for file in files:
            if file.endswith((".cpp", ".h", ".c")):
                with open(os.path.join(root, file), "r", encoding="utf-8", errors="ignore") as f:
                    for c in f.read():
                        if 128 <= ord(c) <= 0xFFFF:
                            chars.add(c)

    # 4. Add Kana and common symbols
    chars.update(chr(c) for c in range(0x3041, 0x3097)) # Hiragana
    chars.update(chr(c) for c in range(0x30A1, 0x30FB)) # Katakana
    chars.update(chr(c) for c in range(0xFF61, 0xFFA0)) # Hankaku Katakana
    chars.update("、。・！？ー〜「」『』（）［］＋−×÷＝％＆￥＄＠＃＊：；…／℃°□")

    return sorted([c for c in chars if ord(c) <= 0xFFFF], key=lambda c: ord(c))

def load_misaki_data():
    if not os.path.exists(MISAKI_HPP):
        return None, None
    with open(MISAKI_HPP, "r", encoding="utf-8", errors="ignore") as f:
        text = f.read()

    p1 = text.find("misaki_uni[] = {")
    p2 = text.find("};", p1)
    uni_entries = [int(x.strip(), 0) for x in text[p1+16:p2].split(",") if x.strip()]

    p3 = text.find("misaki_data[][8] = {")
    p4 = text.find("};", p3)
    data_raw = text[p3+20:p4]
    data_rows = re.findall(r"\{([^}]+)\}", data_raw)
    data_entries = []
    for row in data_rows:
        nums = [int(x.strip(), 0) for x in row.split(",") if x.strip()]
        if len(nums) == 8:
            data_entries.append(nums)

    return uni_entries, data_entries

def load_shinonome_map():
    if not os.path.exists(SHINONOME_BIT):
        return {}
    with open(SHINONOME_BIT, "r", encoding="ascii", errors="ignore") as f:
        text = f.read()

    blocks = re.findall(r"STARTCHAR\s+([0-9a-fA-F]{4})\s+.*?BITMAP\s+([.@\r\n]+)\s+ENDCHAR", text, re.DOTALL)
    shnm_map = {}
    for jis_hex, bitmap_str in blocks:
        val = int(jis_hex, 16)
        b1 = (val >> 8) | 0x80
        b2 = (val & 0xFF) | 0x80
        try:
            u_char = bytes([b1, b2]).decode("euc-jp")
            lines = [x.strip() for x in bitmap_str.split("\n") if x.strip()]
            if len(lines) == 16:
                data = []
                for line in lines:
                    row_val = 0
                    for col in range(min(16, len(line))):
                        if line[col] == '@':
                            row_val |= (0x8000 >> col)
                    data.append((row_val >> 8) & 0xFF)
                    data.append(row_val & 0xFF)
                shnm_map[ord(u_char)] = data
        except Exception:
            pass
    return shnm_map

def render_bitmap(char, target_w, target_h, font, baseline_y, thresh=128):
    img = Image.new("L", (target_w, target_h), 0)
    if char != ' ':
        draw = ImageDraw.Draw(img)
        w = int(font.getlength(char))
        x_off = max(0, (target_w - w) // 2)
        draw.text((x_off, baseline_y), char, font=font, fill=255, anchor='ls')
        img = img.point(lambda p: 1 if p >= thresh else 0, mode='1')

    data = []
    if target_h == 8:
        for y in range(8):
            b = 0
            for x in range(target_w):
                if img.getpixel((x, y)):
                    b |= (0x80 >> x)
            data.append(b)
    elif target_h == 16:
        for y in range(16):
            row_val = 0
            for x in range(target_w):
                if img.getpixel((x, y)):
                    row_val |= (0x8000 >> x)
            data.append((row_val >> 8) & 0xFF)
            data.append(row_val & 0xFF)
    elif target_h == 24:
        for y in range(24):
            row_val = 0
            for x in range(target_w):
                if img.getpixel((x, y)):
                    row_val |= (0x800000 >> x)
            data.append((row_val >> 16) & 0xFF)
            data.append((row_val >> 8) & 0xFF)
            data.append(row_val & 0xFF)
    elif target_h == 32:
        for y in range(32):
            row_val = 0
            for x in range(target_w):
                if img.getpixel((x, y)):
                    row_val |= (0x80000000 >> x)
            data.append((row_val >> 24) & 0xFF)
            data.append((row_val >> 16) & 0xFF)
            data.append((row_val >> 8) & 0xFF)
            data.append(row_val & 0xFF)
    return data

def main():
    print("Collecting characters (including full ASCII)...")
    chars = collect_chars()
    print(f"Total unique characters to generate: {len(chars)}")

    print("Loading Misaki 8x8 font data (Free / Public Domain)...")
    uni_entries, data_entries = load_misaki_data()

    print("Loading Shinonome 16x16 font data (Public Domain)...")
    shinonome_map = load_shinonome_map()
    print(f"Loaded {len(shinonome_map)} glyphs from Shinonome font.")

    font_mplus32 = None
    font_mplus24 = None
    font_mplus16 = None
    font_mplus8 = None
    bl8 = 7
    bl16 = 13
    bl24 = 19
    bl32 = 25
    if os.path.exists(MPLUS_TTF):
        print("Loading M+ font (SIL OFL 1.1) for 24px, 32px and fallbacks...")
        font_mplus32 = ImageFont.truetype(MPLUS_TTF, 27)
        font_mplus24 = ImageFont.truetype(MPLUS_TTF, 21)
        font_mplus16 = ImageFont.truetype(MPLUS_TTF, 15)
        font_mplus8 = ImageFont.truetype(MPLUS_TTF, 8)

    # Sets of characters used per font size
    all_chars_set = set(chars)
    ascii_set = set(chr(c) for c in range(0x20, 0x7F)) & all_chars_set

    i18n_set = set()
    if os.path.exists(I18N_PATH):
        with open(I18N_PATH, "r", encoding="utf-8", errors="ignore") as f:
            for c in f.read():
                if 128 <= ord(c) <= 0xFFFF:
                    i18n_set.add(c)
    i18n_set &= all_chars_set

    kana_set = set(chr(c) for c in range(0x3041, 0x3097)) # Hiragana
    kana_set.update(chr(c) for c in range(0x30A1, 0x30FB)) # Katakana
    kana_set.update(chr(c) for c in range(0xFF61, 0xFFA0)) # Hankaku
    kana_set.update("、。・！？ー〜「」『』（）［］＋−×÷＝％＆￥＄＠＃＊：；…／℃°□")
    kana_set &= all_chars_set

    # Size 1 (8px):  All characters (base font, logs, footers, headers)
    # Size 2 (16px): Menus, UI, keyboard, dialogs (ASCII + i18n + Kana/symbols)
    # Size 3 (24px): Prominent/selected menu items, titles (ASCII + i18n)
    # Size 4 (32px): Clock, timer, big slides, numbers (ASCII only)
    s2_set = ascii_set | i18n_set | kana_set
    s3_set = ascii_set | i18n_set
    s4_set = ascii_set

    print(f"Glyph allocations: Size 1={len(all_chars_set)}, Size 2={len(s2_set)}, Size 3={len(s3_set)}, Size 4={len(s4_set)}")

    glyphs = []
    for c in chars:
        u = ord(c)
        if 0x20 <= u <= 0x7E:
            width = 6  # standard LW=6 for ASCII
        elif 0xFF61 <= u <= 0xFF9F:
            width = 4  # hankaku kana
        else:
            width = 8  # zenkaku

        # 1. 8x8 bitmap (Misaki font for CJK, or M+ rendered) - All characters
        data8 = None
        if uni_entries and u < len(uni_entries):
            idx = uni_entries[u]
            if idx != 0xFFFF and idx < len(data_entries):
                raw8 = data_entries[idx]
                if width == 6:
                    data8 = None
                else:
                    data8 = raw8

        if not data8 and font_mplus8:
            data8 = render_bitmap(c, width, 8, font_mplus8, bl8, thresh=96)

        if not data8:
            data8 = [0] * 8

        # 2. 16x16 bitmap (Shinonome font for CJK, M+ for ASCII/fallbacks) - Only if in s2_set
        target_w16 = width * 2
        data16 = None
        if c in s2_set:
            if width == 8 and u in shinonome_map:
                data16 = shinonome_map[u]
            elif font_mplus16:
                data16 = render_bitmap(c, target_w16, 16, font_mplus16, bl16)
            else:
                data16 = [0] * 32

        # 3. 24x24 bitmap (M+ OFL 21pt centered with baseline anchor) - Only if in s3_set
        target_w24 = width * 3
        data24 = None
        if c in s3_set:
            if font_mplus24:
                data24 = render_bitmap(c, target_w24, 24, font_mplus24, bl24)
            else:
                data24 = [0] * 72

        # 4. 32x32 bitmap (M+ OFL 27pt centered with baseline anchor) - Only if in s4_set
        target_w32 = width * 4
        data32 = None
        if c in s4_set:
            if font_mplus32:
                data32 = render_bitmap(c, target_w32, 32, font_mplus32, bl32)
            else:
                data32 = [0] * 128

        glyphs.append({
            "unicode": u,
            "char": c,
            "width": width,
            "data8": data8,
            "data16": data16,
            "data24": data24,
            "data32": data32
        })

    # Generate Header
    print(f"Writing {OUT_HEADER}...")
    with open(OUT_HEADER, "w", encoding="utf-8") as f:
        f.write("#pragma once\n")
        f.write("// Free Japanese & Latin Font Subset with Per-Size On-Demand Storage Optimization\n")
        f.write("// Size 1 (8px):   Misaki Font (CJK) / M+ (ASCII) [All glyphs]\n")
        f.write("// Size 2 (16px):  Shinonome Font (CJK) / M+ (ASCII) [Menus, UI, Kana]\n")
        f.write("// Size 3 (24px):  M+ Font (SIL OFL 1.1) [Selected menus, ASCII]\n")
        f.write("// Size 4 (32px):  M+ Font (SIL OFL 1.1) [Clock, Timer, ASCII]\n\n")
        f.write("#include <stdint.h>\n")
        f.write("#include <stddef.h>\n\n")
        f.write("struct SubsetGlyph {\n")
        f.write("    uint16_t unicode;\n")
        f.write("    uint8_t width;          // 8 for fullwidth, 6 for ASCII, 4 for halfwidth kana\n")
        f.write("    uint8_t data8[8];       // 8x8 font bitmap (size 1) - always present\n")
        f.write("    const uint8_t* data16;  // 16x16 font bitmap (size 2: 32 bytes) or nullptr\n")
        f.write("    const uint8_t* data24;  // 24x24 font bitmap (size 3: 72 bytes) or nullptr\n")
        f.write("    const uint8_t* data32;  // 32x32 font bitmap (size 4: 128 bytes) or nullptr\n")
        f.write("};\n\n")
        f.write("extern const SubsetGlyph kSubsetGlyphs[];\n")
        f.write("extern const size_t kSubsetGlyphsCount;\n\n")
        f.write("const SubsetGlyph* getSubsetGlyph(uint16_t unicode);\n")

    # Generate CPP
    print(f"Writing {OUT_CPP}...")
    with open(OUT_CPP, "w", encoding="utf-8") as f:
        f.write('#include "subset_font.h"\n\n')
        f.write("// Free Japanese & Latin Font Subset (Per-Size Allocated)\n\n")

        # 1. Output Size 2 bitmap data
        f.write("// --- Size 2 (16px) Bitmaps ---\n")
        for g in glyphs:
            if g["data16"] is not None:
                u_hex = f"{g['unicode']:04X}"
                d16_str = ", ".join(f"0x{b:02X}" for b in g["data16"])
                f.write(f"static const uint8_t g_{u_hex}_d16[32] = {{ {d16_str} }};\n")
        f.write("\n")

        # 2. Output Size 3 bitmap data
        f.write("// --- Size 3 (24px) Bitmaps ---\n")
        for g in glyphs:
            if g["data24"] is not None:
                u_hex = f"{g['unicode']:04X}"
                d24_str = ", ".join(f"0x{b:02X}" for b in g["data24"])
                f.write(f"static const uint8_t g_{u_hex}_d24[72] = {{ {d24_str} }};\n")
        f.write("\n")

        # 3. Output Size 4 bitmap data
        f.write("// --- Size 4 (32px) Bitmaps ---\n")
        for g in glyphs:
            if g["data32"] is not None:
                u_hex = f"{g['unicode']:04X}"
                d32_str = ", ".join(f"0x{b:02X}" for b in g["data32"])
                f.write(f"static const uint8_t g_{u_hex}_d32[128] = {{ {d32_str} }};\n")
        f.write("\n")

        # 4. Output master table
        f.write("// --- Master Glyph Table ---\n")
        f.write("const SubsetGlyph kSubsetGlyphs[] = {\n")
        for g in glyphs:
            u_hex = f"{g['unicode']:04X}"
            d8_str = ", ".join(f"0x{b:02X}" for b in g["data8"])
            p16 = f"g_{u_hex}_d16" if g["data16"] is not None else "nullptr"
            p24 = f"g_{u_hex}_d24" if g["data24"] is not None else "nullptr"
            p32 = f"g_{u_hex}_d32" if g["data32"] is not None else "nullptr"
            comment = g['char'] if g['char'] not in ('\\', '"', '\n', '\r', '\t') else f"U+{g['unicode']:04X}"
            f.write(f"    // '{comment}' (0x{u_hex})\n")
            f.write(f"    {{\n")
            f.write(f"        0x{u_hex}, {g['width']},\n")
            f.write(f"        {{ {d8_str} }},\n")
            f.write(f"        {p16}, {p24}, {p32}\n")
            f.write(f"    }},\n")
        f.write("};\n\n")
        f.write(f"const size_t kSubsetGlyphsCount = sizeof(kSubsetGlyphs) / sizeof(kSubsetGlyphs[0]);\n\n")
        f.write("const SubsetGlyph* getSubsetGlyph(uint16_t unicode) {\n")
        f.write("    int low = 0;\n")
        f.write("    int high = (int)kSubsetGlyphsCount - 1;\n")
        f.write("    while (low <= high) {\n")
        f.write("        int mid = low + (high - low) / 2;\n")
        f.write("        if (kSubsetGlyphs[mid].unicode == unicode) {\n")
        f.write("            return &kSubsetGlyphs[mid];\n")
        f.write("        }\n")
        f.write("        if (kSubsetGlyphs[mid].unicode < unicode) {\n")
        f.write("            low = mid + 1;\n")
        f.write("        } else {\n")
        f.write("            high = mid - 1;\n")
        f.write("        }\n")
        f.write("    }\n")
        f.write("    return nullptr;\n")
        f.write("}\n")

    print(f"Success! Generated {len(glyphs)} glyphs (including full ASCII).")

if __name__ == "__main__":
    main()
