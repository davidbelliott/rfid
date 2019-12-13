#include "buttons.h"
#include "common.h"

#include <avr/io.h>
#include <util/delay.h>

int rd_down;
int rd_up;
int emu_down;
int emu_up;
int lrot;
int rrot;

// encoder stuff

#define ENC_PIN     PINC
#define ENC_A_PIN   PINC0
#define ENC_B_PIN   PINC1

int ENC_CODES[9][2] = {{0,0}, {0,1}, {1,1}, {1,0}, {0,0}, {0,1}, {1,1}, {1,0}, {0,0}};
int ENC_DET_L = 0;
int ENC_DET = 4;
int ENC_DET_R = 8;

int last_enc;


void buttons_init(void) {
    rd_down = TRUE;
    rd_up = FALSE;
    emu_down = FALSE;
    emu_up = FALSE;
    lrot = FALSE;
    rrot = FALSE;

    last_enc = ENC_DET;
}

// Called by interrupt at fixed frequency (~1 ms)
static void debounce_buttons() {
    // TODO: complete
}

// Called by interrupt at fixed frequency (~1 ms)
static void debounce_encoder() {
    // valid encoder signal sequence (AB): 00, 01, 11, 10, 00, 01, 11, 10, 00
    // init last_pos to middle 00

    // read in new pos
    int enc_a = ENC_PIN & (1 << ENC_A_PIN) ? 1 : 0;
    int enc_b = ENC_PIN & (1 << ENC_B_PIN) ? 1 : 0;
    // check if new code == *(last_pos-1) or *(last_pos+1)
    // if so, last_pos = (last_pos-1) or (last_pos+1)
    if((enc_a == ENC_CODES[last_enc-1][0]) && (enc_b == ENC_CODES[last_enc-1][1])) {
        last_enc--;
    }
    if((enc_a == ENC_CODES[last_enc+1][0]) && (enc_b == ENC_CODES[last_enc+1][1])) {
        last_enc++;
    }

    // if curr_pos == table_start THEN reset curr_pos = table_middle ALSO set lrot
    // elif curr_pos == table_end THEN reset curr_pos = table_middle ALSO set rrot
    if(last_enc == ENC_DET_L) {
        last_enc = ENC_DET;
        lrot = TRUE;
    }
    else if(last_enc == ENC_DET_R) {
        last_enc = ENC_DET;
        rrot = TRUE;
    }
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
