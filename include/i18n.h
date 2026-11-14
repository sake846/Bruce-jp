#ifndef __I18N_H__
#define __I18N_H__

#include <Arduino.h>

// Language selection: 1 = Japanese, 0 = English
#ifndef BRUCE_LANG_JA
#define BRUCE_LANG_JA 1
#endif

struct TranslationEntry {
    const char *key;
    const char *ja;
};

// Common translations table
static const TranslationEntry kTranslations[] = {
    // Main Menu Items
    {"WiFi", "Wi-Fi"},
    {"BLE", "Bluetooth (BLE)"},
    {"RF", "無線 (Sub-GHz)"},
    {"2.4GHz", "2.4GHz (NRF)"},
    {"LoRa", "LoRa通信"},
    {"FM Radio", "FMラジオ"},
    {"IR", "赤外線 (IR)"},
    {"Ethernet", "有線LAN"},
    {"GPS", "GPS"},
    {"RFID", "RFID / NFC"},
    {"Files", "ファイル管理"},
    {"Scripts", "スクリプト"},
    {"Clock", "時計 / アラーム"},
    {"Others", "その他ツール"},
    {"Settings", "設定"},
    {"Config", "設定"},
    {"Main Menu", "メインメニュー"},

    // Navigation / Buttons / Footer
    {"Exit", "戻る"},
    {"Back", "戻る"},
    {"Save", "保存"},
    {"Cancel", "中止"},
    {"OK", "OK"},
    {"PREV", "前へ"},
    {"NEXT", "次へ"},
    {"SEL", "決定"},
    {"UP", "上へ"},
    {"DOWN", "下へ"},
    {"[ x ]", "[ 閉じる ]"},

    // Common Actions
    {"Scan", "スキャン"},
    {"Start", "開始"},
    {"Stop", "停止"},
    {"Send", "送信"},
    {"Receive", "受信"},
    {"Clear", "クリア"},
    {"Delete", "削除"},
    {"Rename", "名前変更"},
    {"View", "表示"},
    {"Edit", "編集"},
    {"Clone", "クローン"},
    {"Emulate", "エミュレート"},
    {"Sniff", "スニッファー"},
    {"Attack", "アタック"},
    {"Reboot", "再起動"},
    {"Power Off", "電源オフ"},
    {"Install", "インストール"},

    // WiFi Submenu
    {"Connect to Wifi", "Wi-Fiに接続"},
    {"Start WiFi AP", "Wi-Fi AP起動"},
    {"Turn Off WiFi", "Wi-Fiオフ"},
    {"AP info", "接続AP情報"},
    {"Wifi Atks", "Wi-Fi攻撃"},
    {"Evil Portal", "偽ポータル (Evil)"},
    {"NetCut", "通信切断 (NetCut)"},
    {"Scan APs", "APスキャン"},
    {"Scan Stations", "端末スキャン"},
    {"Beacon Spam", "ビーコン送信"},
    {"Deauth", "切断攻撃 (Deauth)"},
    {"Deauth Flood", "一斉切断攻撃"},
    {"Probe Flood", "Probe送信"},
    {"Sniffer", "パケット傍受"},
    {"RAW Sniffer", "RAWパケット傍受"},
    {"Channel Analyzer", "チャンネル解析"},
    {"Jam Detect", "電波妨害検知"},
    {"Scan Hosts", "ホストスキャン"},
    {"Wireguard", "Wireguard VPN"},
    {"Responder", "Responder"},
    {"Brucegotchi", "ブルースごっち"},
    {"WiFi Pass Recovery", "Wi-Fiパスワード復元"},
    {"Listen TCP", "TCPリッスン"},
    {"Client TCP", "TCPクライアント"},
    {"SOCKS4 Proxy", "SOCKS4プロキシ"},
    {"TelNET", "Telnet"},
    {"SSH", "SSH"},
    {"Change MAC", "MACアドレス変更"},
    {"Add Evil Wifi", "偽AP追加"},
    {"Remove Evil Wifi", "偽AP削除"},
    {"Evil Wifi Settings", "偽AP設定"},

    // BLE Submenu
    {"Bluetooth", "Bluetooth (BLE)"},
    {"Disconnect", "切断"},
    {"Media Cmds", "メディア操作"},
    {"BLE Scan", "BLEスキャン"},
    {"iBeacon", "iBeacon"},
    {"Bad BLE", "BadBLE (キーボード)"},
    {"BLE Keyboard", "BLEキーボード"},
    {"BLE Spam", "BLEスパム"},
    {"Apple Spam", "iOSスパム"},
    {"Windows Spam", "Windowsスパム"},
    {"Samsung Spam", "Samsungスパム"},
    {"Android Spam", "Androidスパム"},
    {"Spam All", "全機種スパム"},
    {"BLE Suite", "BLEスイート"},
    {"Ninebot", "Ninebot操作"},
    {"Presenter mode", "プレゼンター"},
    {"BLE Sniffer", "BLEスニッファー"},

    // RF / Sub-GHz Submenu
    {"Sub-GHz", "Sub-GHz無線"},
    {"Read", "受信・読み取り"},
    {"Read RAW", "RAW受信"},
    {"Transmit", "送信"},
    {"Transmit RAW", "RAW送信"},
    {"Jammer", "ジャマー (妨害)"},
    {"Spectrum", "スペクトラム"},
    {"Pulse Duration", "パルス長"},
    {"RSSI Strength", "RSSI 受信強度"},
    {"Pulse Waveform", "パルス波形"},
    {"Waterfall", "ウォーターフォール"},
    {"Saved", "保存データ"},

    // RFID / NFC Submenu
    {"Read Tag", "タグ読み取り"},
    {"Read UID", "UID読み取り"},
    {"Write Tag", "タグ書き込み"},
    {"Emulate Tag", "タグエミュレート"},
    {"Mifare Classic", "Mifare Classic"},
    {"Ultralight", "Mifare Ultralight"},
    {"NTAG", "NTAG"},
    {"Amiibo", "Amiibo"},

    // IR (Infrared) Submenu
    {"Infrared", "赤外線 (IR)"},
    {"IR Receive", "赤外線受信"},
    {"IR Send", "赤外線送信"},
    {"TV-B-Gone", "TV-B-Gone (全消し)"},
    {"Universal Remote", "汎用リモコン"},

    // Config Menu
    {"Display & UI", "画面・UI設定"},
    {"LED Config", "LED設定"},
    {"Audio Config", "サウンド設定"},
    {"System Config", "システム設定"},
    {"Power", "電源・バッテリー"},
    {"Install App Store", "App Store導入"},
    {"Dev Mode", "開発者モード"},
    {"About", "Bruce情報"},
    {"Brightness", "画面の明るさ"},
    {"Dim Time", "画面減光タイマー"},
    {"Orientation", "画面の向き (回転)"},
    {"UI Color", "UIテーマカラー"},
    {"UI Theme", "テーマ選択"},
    {"Battery", "バッテリー情報"},
    {"Sleep", "スリープ"},
    {"Power Off", "電源オフ"},
    {"Reboot", "再起動"},
};

static const size_t kNumTranslations = sizeof(kTranslations) / sizeof(kTranslations[0]);

inline const char* tr(const char* key) {
#if BRUCE_LANG_JA
    if (!key) return "";
    for (size_t i = 0; i < kNumTranslations; i++) {
        if (strcmp(key, kTranslations[i].key) == 0) {
            return kTranslations[i].ja;
        }
    }
#endif
    return key;
}

inline String tr(const String& key) {
    return String(tr(key.c_str()));
}

inline String safeUtf8Substring(const String& str, size_t maxBytes) {
    if (str.length() <= maxBytes) return str;
    size_t i = maxBytes;
    while (i > 0 && ((uint8_t)str[i] & 0xC0) == 0x80) {
        i--;
    }
    if (i > 0 && ((uint8_t)str[i] & 0x80) != 0) {
        return str.substring(0, i);
    }
    return str.substring(0, maxBytes);
}

#endif // __I18N_H__
