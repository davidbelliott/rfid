#include "slots.h"
#include "common.h"

// Set this bit in the EEPROM contents for every slot that is filled
#define SLOT_FILLED    0x10000000000L

// TODO: make it actually write to/read from EEPROM

unsigned long long slot_data[N_SLOTS] = {0};

int read_slot_data(int slot, unsigned long long *data) {
    unsigned long long val = slot_data[slot];
    //long filled = (val & SLOT_FILLED);
    //if (filled) {
        *data = val;//(val & ~SLOT_FILLED);
        return TRUE;
    /*}
    return FALSE;*/
}

void write_slot_data(int slot, unsigned long long data) {
    slot_data[slot] = data;
}

void clear_slot_data(int slot) {
    slot_data[slot] = 0;
}
