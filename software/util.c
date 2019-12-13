#include "util.h"
#include <avr/io.h>

// data: data to convert, str: dest str, n: num chars
void int_to_hex_str(long data, unsigned char *str, int n) {
    // TODO: complete
}

void toggle_led() {
    int led_pin = (1 << 4);
    if (PORTC & led_pin) {
        PORTC &= ~led_pin;
    } else {
        PORTC |= led_pin;
    }
}

