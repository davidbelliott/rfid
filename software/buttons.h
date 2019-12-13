#ifndef BUTTONS_H
#define BUTTONS_H

#define BUTTON_RD 0
#define BUTTON_EM 1
#define N_BUTTONS 2

void buttons_init();

int button_up(int bt);
int button_down(int bt);
int encoder_lrot();
int encoder_rrot();

#endif // BUTTONS_H
