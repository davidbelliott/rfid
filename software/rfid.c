#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define CARD_DATA_SIZE 64
#define START_SEQ_ONES 9

#define MODE_IDLE   0
#define MODE_READ   1
#define MODE_EMU    2

#define SAMPLE_INTERVAL 0x1000

#define FALSE   0
#define TRUE    1

/*ISR(TIMER1_COMPA_vect) {
    if (PORTC & 0x1) {
        PORTC &= ~0x1;
    } else {
        PORTC |= 0x1;
    }
}*/

int mode;
// If we capture a sequence equivalent to 2 * CARD_DATA_SIZE data bits
// (2 encoded bits == 1 data bit) we are guaranteed to have an uninterrupted
// sequence of all card data bits, which loosens timing constraints on
// decoding portion of the firmware.
int cur_encoded[4 * CARD_DATA_SIZE + 1];
int cur_encoded_idx;

int read_key;
int write_key;

// Falling edge on ICP1 to synchronize sampling with middle of high-low periods
ISR(TIMER1_CAPT_vect) {
    int timer_val = (ICR1H << 8) | ICR1L;
    int offset = SAMPLE_INTERVAL / 2 - timer_val;
    int newval = ((TCNT1H << 8) | TCNT1L) + offset;
    TCNT1H = (newval & 0xFF00) >> 8;
    TCNT1L = (newval & 0x00FF);
}

// There are 64 periods of carrier wave per bit of data, so 32 periods per high/low.
// 
ISR(TIMER1_COMPA_vect) {
    if (cur_encoded_idx == 4 * CARD_DATA_SIZE + 1) {
        return;     // full encoded data sequence already acquired
    }
    cur_encoded[cur_encoded_idx++] = PIND & (1 << PIND6) ? 1 : 0;
}


int manchester_decode(int *encoded, int *data);
int check_parity(int *data);

int handle_encoded_full(int *data) {

    int decoded_data[2 * CARD_DATA_SIZE];
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
    return check_parity(data);
}

// Pin OC0 must be configured as output before calling
void read_start(void) {
    // Set up timer 0 to generate 125 kHz carrier wave
    // toggle OC0 on compare match, no prescaling
    TCCR0 = (1 << WGM01) | (1 << COM00) | (1 << CS00);
    OCR0 = 32;

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
int manchester_decode(int *encoded, int *data) {
    int bit_start = 0;  // the start idx of the first bit period
    for (int i = 0; i < 4 * CARD_DATA_SIZE - 1; i++) {
        if (encoded[i] == encoded[i + 1]) {
            // bit transition must be between i and i + 1
            bit_start = (i + 1) % 2;
            break;
        }
    }
    for (int i = 0; i < 2 * CARD_DATA_SIZE; i++) {
        if (encoded[2 * i + bit_start] == 0 && encoded[2 * i + 1 + bit_start] == 1) {
            data[i] = 1;
        } else if (encoded[2 * i] == 1 && encoded[2 * i + 1] == 0) {
            data[i] = 0;
        } else {
            return FALSE;
        }
    }
    return TRUE;
}

int check_parity(int *data) {
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

void debounce_buttons();

int main (void) {
    sei();  // Set global interrupt enable

    DDRB = 0xFF;
    DDRD &= ~(1 << DDD4);   // Configure PD4 as input (ICP1)
    DDRD &= ~(1 << DDD6);   // Configure PD6 as input (digital data in)
    PORTD &= ~(1 << PORTD6);

    // Turn on external interrupt INT3 (used for read mode synchronization)
    EICRA = (1 << ISC31);   // Falling edge of INT3 generates interrupt request

    mode = MODE_IDLE;
    // infinite loop
    while(1) {

        // Main state machine
        switch (mode) {
            case MODE_IDLE:
                if (read_key) {
                    read_start();
                    mode = MODE_READ;
                }
                break;
            case MODE_READ:
                if (!read_key) {
                    read_end();
                    mode = MODE_IDLE;
                }
                if (cur_encoded_idx == 4 * CARD_DATA_SIZE + 1) {
                    int data[CARD_DATA_SIZE];
                    if (handle_encoded_full(data)) {
                        //write_data(data);
                        mode = MODE_IDLE;
                    } else {
                        cur_encoded_idx = 0;    // try again
                    }
                }
                break;
            default:
                break;  // undefined state; do nothing
        }
    }
    return 0;
}
