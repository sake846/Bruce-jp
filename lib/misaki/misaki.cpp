#include "misaki.hpp"

// UTF-8をUnicodeに変換し、オプションで文字幅を取得する関数
// textはUTF-8文字コード取得後、次の文字を取得する位置に進む
// charWidthはオプションで、指定するとフォントの文字幅を取得できる
// 文字幅は全角(8)または半角(4)のいずれかを返す
// 半角以外は全て全角(8)を返す
uint16_t utf8ToUnicode(const char*& text, uint8_t* charWidth) {
  uint16_t unicode = 0;
  if (charWidth) *charWidth = 8;

  if ((*text & 0x80) == 0) {  // 1バイト (ASCII)
    unicode = *text;
    text++;
    if (charWidth) *charWidth = 4;      // ASCIIのフォント幅(4ドット)
  } else if ((*text & 0xE0) == 0xC0) {  // 2バイト (拡張ラテン)
    unicode = ((*text & 0x1F) << 6) | (text[1] & 0x3F);
    text += 2;

    // ラテン1補助の特定文字を全角(8ドット)扱い
    if (!(unicode == 0x00A7 || unicode == 0x00A8 || unicode == 0x00B0 || unicode == 0x00B1 || unicode == 0x00B4 || unicode == 0x00B6 || unicode == 0x00D7 || unicode == 0x00F7)) {
      if (charWidth) *charWidth = 4;  // その他は半角幅(4ドット)
    }
  } else if ((*text & 0xF0) == 0xE0) {  // 3バイト (日本語)
    unicode = ((*text & 0x0F) << 12) | ((text[1] & 0x3F) << 6) | (text[2] & 0x3F);
    text += 3;

    // 半角カナの判定
    if (charWidth) {
      if (unicode >= 0xFF61 && unicode <= 0xFF9F) {
        *charWidth = 4;  // 半角カナの幅(4ドット)
      }
    }
  } else if ((*text & 0xF8) == 0xF0) {  // 4バイト (絵文字など)
    unicode = ((*text & 0x07) << 18) | ((text[1] & 0x3F) << 12) | ((text[2] & 0x3F) << 6) | (text[3] & 0x3F);
    text += 4;
  } else {
    // 不正なUTF-8シーケンスをスキップ
    text++;
    return 0;
  }

  return unicode;
}

// Unicode値に対応するフォントデータを返す関数
// 見つからない場合はdafaultUnicodeで指定されたフォントデータを返す
// defaultUnicodeを指定しない場合デフォルト値は9633(□)で、見つからない場合はこのフォントデータを返す
const uint8_t* getFontData(uint16_t unicode, uint16_t defaultUnicode) {
  uint16_t index = misaki_uni[unicode];
  if (index != 0xFFFF) {
    return misaki_data[index];
  } else {
    // フォントデータが見つからない場合はデフォルト値のフォントデータを返す
    return misaki_data[misaki_uni[defaultUnicode]];
  }
}