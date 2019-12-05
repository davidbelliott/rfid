#ifndef BUTTONS_H
#define BUTTONS_H

void buttons_init();

int read_button_down();
int read_button_up();
int emu_button_down();
int emu_button_up();
int encoder_lrot();
int encoder_rrot();

#endif // BUTTONS_H
