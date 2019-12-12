#include "buttons.h"
#include "common.h"

int rd_down;
int rd_up;
int emu_down;
int emu_up;
int lrot;
int rrot;

void buttons_init(void) {
    rd_down = TRUE;
    rd_up = FALSE;
    emu_down = FALSE;
    emu_up = FALSE;
    lrot = FALSE;
    rrot = FALSE;
}

// Called by interrupt at fixed frequency (~1 ms)
static void debounce_buttons() {
    // TODO: complete
}

// Called by interrupt at fixed frequency (~1 ms)
static void debounce_encoder() {
    // TODO: complete
}

int read_button_down() {
    int ret = rd_down;
    rd_down = FALSE;
    return ret;
}

int read_button_up() {
    int ret = rd_up;
    rd_up = FALSE;
    return ret;
}

int emu_button_down() {
    int ret = emu_down;
    emu_down = FALSE;
    return ret;
}

int emu_button_up() {
    int ret = emu_up;
    emu_up = FALSE;
    return ret;
}

int encoder_lrot() {
    int ret = lrot;
    lrot = FALSE;
    return ret;
}

int encoder_rrot() {
    int ret = rrot;
    rrot = FALSE;
    return ret;
}
