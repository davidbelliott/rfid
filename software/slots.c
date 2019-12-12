#include "slots.h"
#include "common.h"

// Set this bit in the EEPROM contents for every slot that is filled
#define SLOT_FILLED    0x8000000000000000

// TODO: make it actually write to/read from EEPROM

long slot_data[] = {0, 0, 0, 0, 0, 0, 0, 0};

int read_slot_data(int slot, long *data) {
    long val = slot_data[slot];
    long filled = (val & SLOT_FILLED);
    if (filled) {
        *data = (val & ~SLOT_FILLED);
        return TRUE;
    }
    return FALSE;
}

void write_slot_data(int slot, long data) {
    data |= SLOT_FILLED;
    slot_data[slot] = data;
}

void clear_slot_data(int slot) {
    slot_data[slot] &= ~SLOT_FILLED;
}
