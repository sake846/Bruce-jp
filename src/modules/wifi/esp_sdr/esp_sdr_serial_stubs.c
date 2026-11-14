#include "burst_serial.h"

void burst_serial_init(void) {}
int burst_serial_poll_line(char *line, size_t capacity) {
    (void)line;
    (void)capacity;
    return 0;
}
burst_serial_port_t burst_serial_port(void) { return BURST_SERIAL_USB; }
unsigned burst_serial_baud(void) { return 0; }
bool burst_serial_send(const void *data, size_t size) {
    (void)data;
    (void)size;
    return false;
}
size_t burst_serial_try_send(const void *data, size_t size) {
    (void)data;
    (void)size;
    return 0;
}
bool burst_serial_stop_requested(void) { return false; }
void burst_serial_serve(void (*handle)(char *line)) { (void)handle; }
