#ifndef UTIL_H
#define UTIL_H

#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

void int_to_hex_str(unsigned long long data, unsigned char *str, int n);
void toggle_led();

#endif // UTIL_H
