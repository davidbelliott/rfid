#include "slots.h"
#include "buttons.h"
#include "common.h"
#include "util.h"
#include "lcd.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


#define CARD_DATA_SIZE 64
// If we capture a sequence equivalent to 2 * CARD_DATA_SIZE data bits
// (2 encoded bits == 1 data bit) we are guaranteed to have an uninterrupted
// sequence of all card data bits, eliminating the need to wrap and/or capture
// multiple bit sequences. Since the first data bit could begin with the second
// encoded bit rather than the first, we must also add one to the length.
#define CAPTURE_SEQ_SIZE (4 * CARD_DATA_SIZE + 1)
#define START_SEQ_ONES 9

#define MODE_IDLE           0
#define MODE_READ           1
#define MODE_READ_SUCCESS   2
#define MODE_EMU            3

#define SAMPLE_INTERVAL 0x1000

unsigned char cur_encoded[CAPTURE_SEQ_SIZE];
volatile int cur_encoded_idx;

int main (void);
void blink(void) {
  while(1) //infinite loop
  {
    PORTC = 0xFF; //Turns ON All LEDs
    _delay_ms(1000); //1 second delay
    PORTC= 0x00; //Turns OFF All LEDs
    _delay_ms(1000); //1 second delay
  }
}


// Falling edge on ICP1: synchronize sampling with middle of Manchester periods.
ISR(TIMER1_CAPT_vect) {
    int newval = SAMPLE_INTERVAL / 2;
    TCNT1H = (newval & 0xFF00) >> 8;
    TCNT1L = (newval & 0x00FF);
}

// There are 64 periods of carrier wave per bit of data, so 32 periods per high/low.
ISR(TIMER1_COMPA_vect) {
    if (cur_encoded_idx == CAPTURE_SEQ_SIZE) {
        return;     // full encoded data sequence already acquired
    }
    cur_encoded[cur_encoded_idx++] = PIND & (1 << PIND6) ? 1 : 0;
}

int manchester_decode(unsigned char *encoded, unsigned char *data);
int check_parity(unsigned char *data);

int handle_encoded_bits(long *val) {
    unsigned char data[CARD_DATA_SIZE];
    unsigned char decoded_data[2 * CARD_DATA_SIZE];
    int success = manchester_decode(cur_encoded, decoded_data);
    if (!success) {
        // Try again
        return FALSE;
    }
    // find start sequence in bits
    int n_consecutive_ones = 0;
    int data_start_idx = -1;
    for (int i = 0; i < CARD_DATA_SIZE; i++) {
        if (decoded_data[i]) {
            n_consecutive_ones++;
            if (n_consecutive_ones == START_SEQ_ONES) {
                data_start_idx = i + 1 - START_SEQ_ONES;
                break;
            }
        } else {
            n_consecutive_ones = 0;
        }
    }
    if (data_start_idx == -1) {
        return FALSE; // data start not found in first half of sequence
    }
    // if start sequence found, copy start sequence and following bits to data
    for (int i = 0; i < CARD_DATA_SIZE; i++) {
        data[i] = decoded_data[data_start_idx + i];
    }
    if (!check_parity(data)) {
        return FALSE;
    }
    *val = 0;
    for (int i = 0; i < 40; i++) {
        int bit_src_idx = (i + START_SEQ_ONES + i / 4);
        *val |= data[bit_src_idx] << (39 - i);
    }
    return TRUE;
}

// Pin OC0 must be configured as output before calling
void read_start(void) {
    DDRD &= ~(1 << 4);   // Configure PD4 as input (ICP1)
    DDRD &= ~(1 << 6);   // Configure PD6 as input (digital data in)
    DDRD &= ~(1 << 7);      // DATA_OUT hi-Z
    PORTD &= ~(1 << 6);
    PORTD &= ~(1 << 7);

    // Set up timer 0 to generate 125 kHz carrier wave
    // toggle OC0 on compare match, no prescaling
    TCCR0 = (1 << WGM01) | (1 << COM00) | (1 << CS00);
    OCR0 = 64;

    // Set up timer 1 to trigger sampling interrupts
    // 16 MHz / 125 KHz * 32 periods / sample = every 4096 clocks
    // Also configure input capture interrupts for falling edge, used to
    // synchronize sampling
    // Want 
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS10);
    OCR1AH = (SAMPLE_INTERVAL & 0xFF00) >> 8;
    OCR1AL = SAMPLE_INTERVAL & 0xFF;

    TIMSK = (1 << TICIE1) | (1 << OCIE1A);  // Enable input capture and output compare match interrupts for Timer 1

    // Configure 

    // Start with no encoded bits read
    cur_encoded_idx = 0;
}

void read_end(void) {
    // Disconnect OC0 for normal port operation
    TCCR0 = 0;
    // Disable timer 1 (stop triggering sampling interrupts and input capture)
    TCCR1B = 0;

    TIMSK = 0;  // Disable timer interrupts
}

void emu_start(void) {
    
}

void emu_end(void) {

}

// Encode a bit sequence of CARD_DATA_SIZE bits into a high/low Manchester
// sequence of 2 * CARD_DATA_SIZE bits.
void manchester_encode(int *data, int *encoded) {
    for (int i = 0; i < CARD_DATA_SIZE; i++) {
        if (data[i]) {
            encoded[2 * i] = 0;
            encoded[2 * i + 1] = 1;
        } else {
            encoded[2 * i] = 1;
            encoded[2 * i + 1] = 0;
        }
    }
}

// Decode a Manchester-encoded bit sequence of 4 * CARD_DATA_SIZE bits into a 
// decoded sequence of 2 * CARD_DATA_SIZE bits
int manchester_decode(unsigned char *encoded, unsigned char *data) {
    int bit_start = 0;  // the start idx of the first bit period
    for (int i = 0; i < CAPTURE_SEQ_SIZE - 1; i++) {
        if (encoded[i] == encoded[i + 1]) {
            // bit transition must be between i and i + 1
            bit_start = (i + 1) % 2;
            break;
        }
    }
    for (int i = 0; i < 2 * CARD_DATA_SIZE; i++) {
        if (encoded[2 * i + bit_start] == 1 && encoded[2 * i + 1 + bit_start] == 0) {
            data[i] = 1;
        } else if (encoded[2 * i + bit_start] == 0 && encoded[2 * i + 1 + bit_start] == 1) {
            data[i] = 0;
        } else {
            return FALSE;
        }
    }
    return TRUE;
}

int check_parity(unsigned char *data) {
    // Check start sequence
    for (int i = 0; i < START_SEQ_ONES; i++) {
        if (data[i] != 1) {
            return FALSE;
        }
    }
    // Check all row parity bits
    for (int i = 0; i < 10; i++) {
        int idx = START_SEQ_ONES + 5 * i;
        if ((data[idx] + data[idx + 1] + data[idx + 2] + data[idx + 3]) % 2 != data[idx + 4]) {
            return FALSE;
        }
    }
    // Check all col parity bits
    for (int i = 1; i < 4; i ++) {
        int a = 0;
        for (int j = 0; j < 10; j++) {
            a += data[START_SEQ_ONES + 5 * j + i];
        }
        if (a % 2 != data[START_SEQ_ONES + 50 + i]) {
            return FALSE;
        }
    }
    // Check stop bit
    if (data[CARD_DATA_SIZE - 1]) {
        return FALSE;
    }
    return TRUE;
}

// data: data to convert, str: dest str, n: num chars
void int_to_hex_str(long data, unsigned char *str, int n) {
    for (int i = 0; i < n; i++) {

    }
}

void disp_slot(int slot) {
    unsigned char slot_str[] = "Slot x:";
    slot_str[5] = slot + 0x30;
    lcd_display(0, 0, slot_str);
    unsigned char slot_data[] = "DEADBEEF";
    lcd_display(1, 1, slot_data);
    long slot_data;
    int exists = read_slot_data(slot, &slot_data);
    if (exists) {
        unsigned char hex_str[11];
        int_to_hex_str(slot_data, hex_str, 11);
        lcd_show(hex_str);
    }
}

int main (void) {
    sei();  // Set global interrupt enable
    int slot = 0;
    int mode = MODE_IDLE;
    DDRB = 0xFF;
    DDRC = 0xFF;

    lcd_init();
    buttons_init();

    // Turn on external interrupt INT3 (used for read mode synchronization)
    //EICRA = (1 << ISC31);   // Falling edge of INT3 generates interrupt request

    slot = 0;
    disp_slot(slot);
    // infinite loop
    while(1) {
        // Get any button/encoder events that happened since last check
        int rd_down = button_down(BUTTON_RD);
        int rd_up = button_up(BUTTON_RD);
        int emu_down = button_down(BUTTON_EM);
        int emu_up = button_up(BUTTON_EM);
        int lrot = encoder_lrot();
        int rrot = encoder_rrot();

        // Main state machine
        switch (mode) {
            case MODE_IDLE:
                if (rd_down) {
                    read_start();
                    mode = MODE_READ;
                } else if (emu_down) {
                    emu_start();
                    mode = MODE_EMU;
                }
                break;
            case MODE_READ:
                if (rd_up) {
                    read_end();
                    mode = MODE_IDLE;
                }
                if (cur_encoded_idx == 4 * CARD_DATA_SIZE + 1) {

                    read_end();
                    long val;   // MODE_DECODE
                    if (handle_encoded_bits(&val)) {
                        write_slot_data(slot, val);
                        PORTC = 0xFF;
                        mode = MODE_READ_SUCCESS;
                    } else {
                        read_start();
                    }
                }
                break;
            case MODE_READ_SUCCESS:
                if (rd_up) {
                    PORTC = 0x00;
                    mode = MODE_IDLE;
                }
                break;
            case MODE_EMU:
                if (emu_up) {
                    emu_end();
                    mode = MODE_IDLE;
                }
                break;
            default:
                break;  // undefined state; do nothing
        }
        // Slot state machine: every state (slot index) has the same logic
        if (lrot) {
            lrot = FALSE;
            slot = (slot == 0 ? N_SLOTS - 1 : slot - 1);
            disp_slot(slot);
        } else if (rrot) {
            rrot = FALSE;
            slot = (slot == N_SLOTS - 1 ? 0 : slot + 1);
            disp_slot(slot);
        }
    }
    return 0;
}
