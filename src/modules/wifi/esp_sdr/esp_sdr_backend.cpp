#include "standalone_display_backend.h"
#include <globals.h>
#include "core/utils.h"

extern "C" void esp_sdr_bruce_display_row(unsigned y, const uint16_t *pixels) {
    tft.pushImage(0, y, 320, 1, const_cast<uint16_t *>(pixels));
}

extern "C" unsigned esp_sdr_bruce_poll_controls(m5stack_controls_t *controls, unsigned mhz) {
    if (check(EscPress)) esp_sdr_bruce_request_exit();

    int32_t steps = drainRotarySteps();
    if (steps) {
        check(PrevPress);
        check(NextPress);
        check(UpPress);
        check(DownPress);
        controls->last_input = millis();
        while (steps) {
            mhz = m5stack_menu_adjust(controls, mhz, steps < 0 ? -1 : 1);
            steps += steps < 0 ? 1 : -1;
        }
        PrevPress = NextPress = UpPress = DownPress = false;
    }
    if (check(SelPress)) {
        controls->last_input = millis();
        m5stack_menu_move(controls, LongPress);
    }
    return mhz;
}

extern "C" int esp_sdr_bruce_battery_percent(void) {
    return getBattery();
}
