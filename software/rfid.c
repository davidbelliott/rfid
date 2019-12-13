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
#define EMU_SEQ_SIZE (2 * CARD_DATA_SIZE)
#define START_SEQ_ONES 9

#define MODE_IDLE           0
#define MODE_READ           1
#define MODE_READ_SUCCESS   2
#define MODE_EMU            3

#define SAMPLE_INTERVAL 0x1000

volatile int mode;

unsigned char cur_encoded[CAPTURE_SEQ_SIZE];
volatile int cur_encoded_idx;

unsigned char cur_emu_encoded[EMU_SEQ_SIZE];
volatile int cur_emu_idx;

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
    switch (mode) {
        case MODE_READ:
            if (cur_encoded_idx == CAPTURE_SEQ_SIZE) {
                return;     // full encoded data sequence already acquired
            }
            cur_encoded[cur_encoded_idx++] = PIND & (1 << PIND6) ? 1 : 0;
            break;
        case MODE_EMU:
            if (!cur_emu_encoded[cur_emu_idx]) {
                PORTD |= (1 << 7);
            } else {
                PORTD &= ~(1 << 7);
            }
            cur_emu_idx = (cur_emu_idx == EMU_SEQ_SIZE - 1 ? 0 : cur_emu_idx + 1);
            break;
        default:
            break;
    }
}

void manchester_encode(unsigned char *data, unsigned char *encoded);
int manchester_decode(unsigned char *encoded, unsigned char *data);
int check_parity(unsigned char *data);

void prepare_encoded_bits(unsigned long long val) {
    unsigned char data[CARD_DATA_SIZE];
    for (int i = 0; i < CARD_DATA_SIZE; i++) {
        data[i] = 0;
    }
    for (int i = 0; i < START_SEQ_ONES; i++) {
        data[i] = 1;
    }
    // Populate data bits
    for (int i = 0; i < 40; i++) {
        int bit_dest_idx = (i + START_SEQ_ONES + i / 4);
        data[bit_dest_idx] = val & (1ULL << (39 - i)) ? 1 : 0;
    }
    // Populate row parity bits
    for (int i = 0; i < 10; i++) {
        int idx = START_SEQ_ONES + 5 * i;
        data[idx + 4] = (data[idx] + data[idx + 1] + data[idx + 2] + data[idx + 3]) % 2;
    }
    // Populate col parity bits
    for (int i = 0; i < 4; i++) {
        int a = 0;
        for (int j = 0; j < 10; j++) {
            a += data[START_SEQ_ONES + 5 * j + i];
        }
        data[START_SEQ_ONES + 50 + i] = a % 2;
    }
    // Populate stop bit
    data[CARD_DATA_SIZE - 1] = 0;
    manchester_encode(data, cur_emu_encoded);
}

int handle_encoded_bits(unsigned long long *val) {
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
        *val |= (unsigned long long)data[bit_src_idx] << (39 - i);
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

    // Start with no encoded bits read
    cur_encoded_idx = 0;

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

    TIMSK |= (1 << TICIE1) | (1 << OCIE1A);  // Enable input capture and output compare match interrupts for Timer 1

}

void read_end(void) {
    // Disconnect OC0 for normal port operation
    TCCR0 = 0;
    // Disable timer 1 (stop triggering sampling interrupts and input capture)
    TCCR1B = 0;
    TIMSK &= ~((1 << TICIE1) | (1 << OCIE1A));  // Disable timer interrupts
}

void emu_start(unsigned long long val) {
    DDRD |= (1 << 7);   // Configure PD7 as output (DATA_OUT)
    for (int i = 0; i < EMU_SEQ_SIZE; i++) {
        cur_emu_encoded[i] = i % 2;
    }
    prepare_encoded_bits(val);
    cur_emu_idx = 0;

    // Set up timer 1 to trigger sampling interrupts
    // 16 MHz / 125 KHz * 32 periods / sample = every 4096 clocks
    // Also configure input capture interrupts for falling edge, used to
    // synchronize sampling
    // Want 
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS10);
    OCR1AH = (SAMPLE_INTERVAL & 0xFF00) >> 8;
    OCR1AL = SAMPLE_INTERVAL & 0xFF;

    TIMSK |= (1 << OCIE1A);  // Enable input capture and output compare match interrupts for Timer 1
}

void emu_end(void) {
    // Disable timer 1
    TCCR1B = 0;
    TIMSK &= ~(1 << OCIE1A);  // Disable timer interrupts
}

// Encode a bit sequence of CARD_DATA_SIZE bits into a high/low Manchester
// sequence of 2 * CARD_DATA_SIZE bits.
void manchester_encode(unsigned char *data, unsigned char *encoded) {
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

void disp_slot(int slot) {
    lcd_clear();
    unsigned char slot_str[] = "Slot x:";
    slot_str[5] = slot + 0x30;
    lcd_display(0, 0, slot_str);
    unsigned long long slot_data;
    int exists = read_slot_data(slot, &slot_data);
    if (exists) {
        unsigned char hex_str[13] = "0x";
        int_to_hex_str(slot_data, hex_str + 2, 10);
        lcd_display(1, 0, hex_str);
    }
}

int main (void) {
    sei();  // Set global interrupt enable
    int slot = 0;
    mode = MODE_IDLE;
    DDRB = 0xFF;
    DDRC |= (1 << 4);   // stat (active low LED)
    PORTC |= (1 << 4);

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

        /*if (rd_up) {
            toggle_led();
        }*/

        // Main state machine
        switch (mode) {
            case MODE_IDLE:
                if (rd_down) {
                    read_start();
                    mode = MODE_READ;
                } else if (emu_down) {
                    unsigned long long val;
                    read_slot_data(slot, &val);
                    emu_start(val);
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
                    unsigned long long val;   // MODE_DECODE
                    if (handle_encoded_bits(&val)) {
                        write_slot_data(slot, val);
                        disp_slot(slot);
                        PORTC &= ~(1 << 4);
                        read_end();
                        mode = MODE_READ_SUCCESS;
                    } else {
                        read_start();
                    }
                }
                break;
            case MODE_READ_SUCCESS:
                if (rd_up) {
                    PORTC |= (1 << 4);
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
            slot = (slot == 0 ? N_SLOTS - 1 : slot - 1);
            disp_slot(slot);
        } else if (rrot) {
            slot = (slot == N_SLOTS - 1 ? 0 : slot + 1);
            disp_slot(slot);
        }
    }
    return 0;
}
