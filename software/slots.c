#include "slots.h"
#include "common.h"
#include <avr/eeprom.h>

// Set this bit in the EEPROM contents for every slot that is filled
#define SLOT_FILLED    0x10000000000L

#define EEPROM_ADDR 0x0

unsigned long long slot_data[N_SLOTS] = {0};

void slots_init() {
    eeprom_read_block((void*) slot_data, (void*)EEPROM_ADDR, sizeof(unsigned long long)*N_SLOTS);
}

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
    eeprom_update_block((void*)slot_data, (void*)EEPROM_ADDR, sizeof(unsigned long long)*N_SLOTS);
}

void clear_slot_data(int slot) {
    slot_data[slot] = 0;
    eeprom_update_block((void*)slot_data, (void*)EEPROM_ADDR, sizeof(unsigned long long)*N_SLOTS);
}
