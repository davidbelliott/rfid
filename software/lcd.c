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

void lcd_show(unsigned char *text)	// 16 chars
{
    send_cmd(CLR_DISP_CMD);	// clear display command - also sets DDRAM address to 0
    _delay_ms(10);
    // hold for busy flag
    // set ddram address? (or just set to home - this unnecessary since not shifting)
    // hold for busy flag
    send_data(text);	// write data to DDRAM
}

void init_lcd()
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
    send_cmd(0x70); // contrast low
    _delay_ms(1);
    send_cmd(0x50); // contrast high
    _delay_ms(1);
    send_cmd(0x6F);
    _delay_ms(200);
    send_cmd(0x0F); //display on
    _delay_ms(1);
    send_cmd(0x01);
    _delay_ms(350);
    send_cmd(0x06);
    _delay_ms(1);
    send_cmd(0x02);
    _delay_ms(1);
}
