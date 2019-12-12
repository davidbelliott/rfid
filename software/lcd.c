#include "lcd.h"
#include "twi.h"
#include <avr/io.h>
#include <util/delay.h>

void send_cmd(unsigned char cmd) {
    unsigned char data[2];
    data[0] = CMD_CTL;
    data[1] = cmd;
    twi_write(LCD_I2C + I2C_WR, data, 2, 0x0);
    twi_wait();
}

void send_data(unsigned char *text) {
    unsigned char data[17];
    data[0] = DATA_CTL;
    int len = 0;
    for(len=0; len<16 && text[len]; len++)
        data[len+1] = text[len];	
    twi_write(LCD_I2C + I2C_WR, data, len + 1, 0x0);
    twi_wait();
}

void wait_busy() {
    // TODO: implement
}

void lcd_clear() {
    send_cmd(CLR_DISP_CMD);
}

void lcd_display(int r, int c, unsigned char *str) {
    if (r >= 0) {
        int addr = r + (c ? 0x40 : 0);
        send_cmd(0x80 | addr);
        _delay_ms(1);
    }
    send_data(str);
}

void lcd_init()
{
    // pull active low reset high
    DDRD |= (1 << 2);
    PORTD |= (1 << 2);

    twi_init();

    // wait 40ms
    _delay_ms(40);

    send_cmd(0x38);
    _delay_ms(1);
    send_cmd(0x39);
    _delay_ms(1);
    send_cmd(0x14); // internal osc freq
    _delay_ms(1);
    send_cmd(0x78); // contrast low
    _delay_ms(1);
    send_cmd(0x54); // contrast high
    _delay_ms(1);
    send_cmd(0x6F);
    _delay_ms(200);
    send_cmd(0x0C); //display on
    _delay_ms(1);
    send_cmd(0x01);
    _delay_ms(350);
    send_cmd(0x06);
    _delay_ms(1);
    send_cmd(0x02);
    _delay_ms(1);
}
