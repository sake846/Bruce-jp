#include "esp_sdr.h"
#include "core/display.h"
#include "core/utils.h"
#include "core/wifi/wifi_common.h"
#include "modules/ble/ble_common.h"
#include <globals.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern "C" {
bool esp_sdr_bruce_start(void);
void esp_sdr_bruce_poll(void);
bool esp_sdr_bruce_exit_requested(void);
void esp_sdr_bruce_stop(void);
}

void espSdrMenu(void) {
    if (WiFi.getMode() != WIFI_MODE_NULL || wifiConnected) {
        displayError("Turn off WiFi first", true);
        return;
    }
    if (BLEConnected) {
        displayError("Disconnect BLE first", true);
        return;
    }

    stopBLEStack();
    ensureWifiPlatform();
    if (!esp_sdr_bruce_start()) {
        esp_sdr_bruce_stop();
        WiFi.mode(WIFI_OFF);
        displayError("ESP-SDR init failed", true);
        return;
    }

    while (!esp_sdr_bruce_exit_requested()) {
        esp_sdr_bruce_poll();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    esp_sdr_bruce_stop();
    WiFi.mode(WIFI_OFF);
    tft.fillScreen(bruceConfig.bgColor);
    returnToMenu = true;
}
