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

    # 1. i18n.h
    if os.path.exists(I18N_PATH):
        with open(I18N_PATH, "r", encoding="utf-8", errors="ignore") as f:
            for c in f.read():
                if 128 <= ord(c) <= 0xFFFF:
                    chars.add(c)

    # 2. Scan src for non-ascii
    src_dir = os.path.join(REPO_ROOT, "src")
    for root, _, files in os.walk(src_dir):
        for file in files:
            if file.endswith((".cpp", ".h", ".c")):
                with open(os.path.join(root, file), "r", encoding="utf-8", errors="ignore") as f:
                    for c in f.read():
                        if 128 <= ord(c) <= 0xFFFF:
                            chars.add(c)

    # 3. Add Kana and common symbols
    chars.update(chr(c) for c in range(0x3041, 0x3097)) # Hiragana
    chars.update(chr(c) for c in range(0x30A1, 0x30FB)) # Katakana
    chars.update(chr(c) for c in range(0xFF61, 0xFFA0)) # Hankaku Katakana
    chars.update("、。・！？ー〜「」『』（）［］＋−×÷＝％＆￥＄＠＃＊：；…／℃°□")

    return sorted([c for c in chars if 128 <= ord(c) <= 0xFFFF], key=lambda c: ord(c))

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

def render_16x16_fallback(char, font, is_hankaku):
    w = 8 if is_hankaku else 16
    img = Image.new("1", (w, 16), 0)
    draw = ImageDraw.Draw(img)
    draw.text((0, 0), char, font=font, fill=1)
    data = []
    for y in range(16):
        row_val = 0
        for x in range(w):
            if img.getpixel((x, y)):
                row_val |= (0x8000 >> x)
        data.append((row_val >> 8) & 0xFF)
        data.append(row_val & 0xFF)
    return data

def main():
    print("Collecting characters...")
    chars = collect_chars()
    print(f"Total unique characters to generate: {len(chars)}")

    print("Loading Misaki 8x8 font data (Free / Public Domain)...")
    uni_entries, data_entries = load_misaki_data()

    print("Loading Shinonome 16x16 font data (Public Domain)...")
    shinonome_map = load_shinonome_map()
    print(f"Loaded {len(shinonome_map)} glyphs from Shinonome font.")

    font_mplus16 = None
    font_mplus8 = None
    if os.path.exists(MPLUS_TTF):
        print("Loading M+ font (SIL OFL 1.1) for halfwidth/fallback...")
        font_mplus16 = ImageFont.truetype(MPLUS_TTF, 16)
        font_mplus8 = ImageFont.truetype(MPLUS_TTF, 8)

    glyphs = []
    for c in chars:
        u = ord(c)
        is_hankaku = (0xFF61 <= u <= 0xFF9F)
        width = 4 if is_hankaku else 8

        # 1. 8x8 bitmap (Misaki font)
        data8 = None
        if uni_entries and u < len(uni_entries):
            idx = uni_entries[u]
            if idx != 0xFFFF and idx < len(data_entries):
                data8 = data_entries[idx]

        if not data8 and font_mplus8:
            img8 = Image.new("1", (width, 8), 0)
            draw8 = ImageDraw.Draw(img8)
            draw8.text((0, 0), c, font=font_mplus8, fill=1)
            data8 = []
            for y in range(8):
                b = 0
                for x in range(width):
                    if img8.getpixel((x, y)):
                        b |= (0x80 >> x)
                data8.append(b)

        if not data8:
            data8 = [0] * 8

        # 2. 16x16 bitmap (Shinonome font -> M+ OFL fallback)
        data16 = shinonome_map.get(u)
        if not data16:
            if font_mplus16:
                data16 = render_16x16_fallback(c, font_mplus16, is_hankaku)
            else:
                data16 = [0] * 32

        glyphs.append({
            "unicode": u,
            "char": c,
            "width": width,
            "data8": data8,
            "data16": data16
        })

    # Generate Header
    print(f"Writing {OUT_HEADER}...")
    with open(OUT_HEADER, "w", encoding="utf-8") as f:
        f.write("#pragma once\n")
        f.write("// Free Japanese Font Subset\n")
        f.write("// 16x16: Shinonome Font (Public Domain) / M+ Font (SIL OFL 1.1)\n")
        f.write("// 8x8:   Misaki Font (Free / Public Domain)\n\n")
        f.write("#include <stdint.h>\n")
        f.write("#include <stddef.h>\n\n")
        f.write("struct SubsetGlyph {\n")
        f.write("    uint16_t unicode;\n")
        f.write("    uint8_t width;      // 8 for fullwidth, 4 for halfwidth\n")
        f.write("    uint8_t data8[8];   // 8x8 font bitmap (1 byte per row)\n")
        f.write("    uint8_t data16[32]; // 16x16 font bitmap (2 bytes per row, MSB on left)\n")
        f.write("};\n\n")
        f.write("extern const SubsetGlyph kSubsetGlyphs[];\n")
        f.write("extern const size_t kSubsetGlyphsCount;\n\n")
        f.write("const SubsetGlyph* getSubsetGlyph(uint16_t unicode);\n")

    # Generate CPP
    print(f"Writing {OUT_CPP}...")
    with open(OUT_CPP, "w", encoding="utf-8") as f:
        f.write('#include "subset_font.h"\n\n')
        f.write("// Fully open-source / public domain Japanese font glyph dataset\n")
        f.write("const SubsetGlyph kSubsetGlyphs[] = {\n")
        for g in glyphs:
            u_hex = f"0x{g['unicode']:04X}"
            d8_str = ", ".join(f"0x{b:02X}" for b in g["data8"])
            d16_str = ", ".join(f"0x{b:02X}" for b in g["data16"])
            comment = g['char'] if g['char'] not in ('\\', '"', '\n', '\r') else f"U+{g['unicode']:04X}"
            f.write(f"    // '{comment}' ({u_hex})\n")
            f.write(f"    {{\n")
            f.write(f"        {u_hex}, {g['width']},\n")
            f.write(f"        {{ {d8_str} }},\n")
            f.write(f"        {{ {d16_str} }}\n")
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

    print(f"Success! Generated {len(glyphs)} glyphs from 100% free fonts.")

if __name__ == "__main__":
    main()
