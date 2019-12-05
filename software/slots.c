#include "slots.h"
#include "common.h"

// Set this bit in the EEPROM contents for every slot that is filled
#define SLOT_FILLED    0x8000000000000000

int read_slot_data(int slot, long *data) {
    // TODO: complete
    return FALSE;
}

void write_slot_data(int slot, long data) {
    data |= SLOT_FILLED;
    // TODO: complete
}

void clear_slot_data(int slot) {

}
