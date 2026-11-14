#pragma once
#include <stdint.h>

#if CONFIG_ESP_SDR_BRUCE
#include "standalone_display_controls.h"
#ifdef __cplusplus
extern "C" {
#endif
void esp_sdr_bruce_display_row(unsigned y, const uint16_t *pixels);
unsigned esp_sdr_bruce_poll_controls(m5stack_controls_t *controls, unsigned mhz);
void esp_sdr_bruce_request_exit(void);
int esp_sdr_bruce_battery_percent(void);
#ifdef __cplusplus
}
#endif
static inline void standalone_display_backend_init(void) {}
static inline unsigned standalone_display_backend_buttons(void) { return 0; }
static inline int standalone_display_backend_encoder(void) { return 0; }
static inline void standalone_display_backend_row(unsigned y, const uint16_t *pixels) {
    esp_sdr_bruce_display_row(y, pixels);
}
static inline void standalone_display_backend_flush(void) {}
static inline void standalone_display_backend_power_off(void) {
    esp_sdr_bruce_request_exit();
}
#else
#if CONFIG_ESP_SDR_M5STACK_BASIC && CONFIG_ESP_SDR_M5STACK_LCD_ILI9341
#include "standalone_device_m5stack_basic_ili9341.h"
static inline void standalone_display_backend_init(void) {
    standalone_device_m5stack_basic_ili9341_init();
}
static inline unsigned standalone_display_backend_buttons(void) {
    return standalone_device_m5stack_basic_ili9341_buttons();
}
static inline int standalone_display_backend_encoder(void) { return 0; }
static inline void standalone_display_backend_row(unsigned y, const uint16_t *pixels) {
    standalone_device_m5stack_basic_ili9341_row(y, pixels);
}
static inline void standalone_display_backend_flush(void) {
    standalone_device_m5stack_basic_ili9341_flush();
}
static inline void standalone_display_backend_power_off(void) {
    standalone_device_m5stack_basic_ili9341_power_off();
}
#elif CONFIG_ESP_SDR_M5STACK
#include "standalone_device_m5stack.h"
static inline void standalone_display_backend_init(void) {
    m5stack_device_adapter_init();
}
static inline unsigned standalone_display_backend_buttons(void) {
    return m5stack_device_adapter_buttons();
}
static inline int standalone_display_backend_encoder(void) {
    return m5stack_device_adapter_encoder();
}
static inline void standalone_display_backend_row(unsigned y, const uint16_t *pixels) {
    m5stack_device_adapter_row(y, pixels);
}
static inline void standalone_display_backend_flush(void) {
    m5stack_device_adapter_flush();
}
static inline void standalone_display_backend_power_off(void) {
    m5stack_device_adapter_power_off();
}
#elif CONFIG_ESP_SDR_LILYGO || CONFIG_ESP_SDR_ATS_MINI
#include "standalone_device_lilygo_ats_mini.h"
static inline void standalone_display_backend_init(void) {
    standalone_device_lilygo_ats_mini_init();
}
static inline unsigned standalone_display_backend_buttons(void) { return 0; }
static inline int standalone_display_backend_encoder(void) { return 0; }
static inline void standalone_display_backend_row(unsigned y, const uint16_t *pixels) {
    standalone_device_lilygo_ats_mini_display_row(y, pixels);
}
static inline void standalone_display_backend_flush(void) {
    standalone_device_lilygo_ats_mini_display_flush();
}
static inline void standalone_display_backend_power_off(void) {
    standalone_device_lilygo_ats_mini_power_off();
}
#else
#error Select a standalone display device
#endif
