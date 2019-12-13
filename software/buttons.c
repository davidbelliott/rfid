#include "buttons.h"
#include "common.h"
#include "util.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// encoder stuff

#define ENC_PIN     PINC
#define ENC_A_PIN   PINC0
#define ENC_B_PIN   PINC1

int ENC_CODES[9][2] = {{0,0}, {0,1}, {1,1}, {1,0}, {0,0}, {0,1}, {1,1}, {1,0}, {0,0}};
int ENC_DET_L = 0;
int ENC_DET = 4;
int ENC_DET_R = 8;

int last_enc;

// button stuff

#define DEBOUNCE_MS 20

int bt_state[N_BUTTONS];
int until_debounced[N_BUTTONS];
volatile int bt_down[N_BUTTONS];
volatile int bt_up[N_BUTTONS];
int bt_pin[N_BUTTONS] = {2, 3};
int lrot;
int rrot;

static void debounce_buttons();
static void debounce_encoder();

ISR(TIMER2_COMP_vect) {
    debounce_buttons();
    debounce_encoder();
}

void buttons_init(void) {

    DDRC &= ~(0xF);    // PC0-3 inputs
    for (int i = 0; i < N_BUTTONS; i++) {
        bt_state[i] = 0;
        until_debounced[i] = 0;
        bt_down[i] = FALSE;
        bt_up[i] = FALSE;
        PORTC |= (1 << bt_pin[i]);  // configure pull-up
    }
    lrot = FALSE;
    rrot = FALSE;
    last_enc = ENC_DET;
    
    TCCR2 = 0b00001100;
    OCR2 = 60;
    TIMSK |= (1 << OCIE2);
}

// 0 = rd, 1 = emu
// ret zero if not pushed, nonzero if pushed
static int get_button(int bt) {
    return PINC & (1 << bt_pin[bt]) ? FALSE : TRUE;
}

// Called by interrupt at fixed frequency (~1 ms)
static void debounce_buttons() {
   for (int i = 0; i < N_BUTTONS; i++) {
        int bt_val = get_button(i);
        if (bt_val != bt_state[i]) {
            until_debounced[i] = DEBOUNCE_MS;
            bt_state[i] = bt_val;
        } else if (until_debounced[i] > 0) {
            until_debounced[i]--;
            if (until_debounced[i] == 0) {
                if (bt_val == 0) {
                    bt_up[i] = TRUE;
                } else {
                    bt_down[i] = TRUE;
                }
            }
        }
    }
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

int button_down(int bt) {
    cli();
    int ret = bt_down[bt];
    bt_down[bt] = FALSE;
    sei();
    return ret;
}

int button_up(int bt) {
    cli();
    int ret = bt_up[bt];
    bt_up[bt] = FALSE;
    sei();
    return ret;
}

int encoder_lrot() {
    cli();
    int ret = lrot;
    lrot = FALSE;
    sei();
    return ret;
}

int encoder_rrot() {
    cli();
    int ret = rrot;
    rrot = FALSE;
    sei();
    return ret;
}
