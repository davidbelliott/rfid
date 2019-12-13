#include "util.h"
#include <avr/io.h>

unsigned char get_hex_dig(int n) {
    if (n < 10) {
        return (48 + n);
    }
    return (55 + n);
}

// data: data to convert, str: dest str, n: num digit places
void int_to_hex_str(unsigned long long data, unsigned char *str, int n) {
    int null_pos = MIN(n, 16);
    for (int i = 0; i < null_pos; i++) {
        str[null_pos - 1 - i] = get_hex_dig(data % 16);
        data /= 16;
    }
    str[null_pos] = 0;
}

void toggle_led() {
    int led_pin = (1 << 4);
    if (PORTC & led_pin) {
        PORTC &= ~led_pin;
    } else {
        PORTC |= led_pin;
    }
}

