#ifndef SLOTS_H
#define SLOTS_H

#define N_SLOTS 8

int read_slot_data(int slot, unsigned long long *data);
void write_slot_data(int slot, unsigned long long data);
void clear_slot_data(int slot);

#endif // SLOTS_H
