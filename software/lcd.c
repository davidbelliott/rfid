#include "lcd.h"

// https://github.com/tomozh/arduino_ST7032


void i2c_out(unsigned char j)
{
	int n;
	unsigned char d;
	d = j;

	for(n=0; n<8; n++) {
		if((d & 0x80) == 0x80)
			SDA = 1;
		else
			SDA = 0;

		d = (d<<1);
		SCL = 0;
		SCL = 1;
		SCL = 0;
	}

	SCL = 1;
	while(SDA == 1) {	// gets caught here if no slave acknowledge
		SCL = 0;
		SCL = 1;
	}
	SCL = 0;
}

void i2c_start(void)
{
	SCL = 1;
	SDA = 1;
	SDA = 0;
	SCL = 0;
}

void i2c_stop(void)
{
	SDA = 0;
	SCL = 0;
	SCL = 1;
	SDA = 1;
}

void i2c_begin_tx(unsigned char slave_addr) {
	i2c_start();
	i2c_out((slave_addr << 1) | I2C_WR);
}

void send_cmd(unsigned char cmd) {
	i2c_begin_tx(LCD_I2C);
	i2c_out(CMD_CTL);
	i2c_out(cmd);
	i2c_stop();

	// add 30us delay?
}

void send_data(unsigned char *text) {
	i2c_begin_tx(LCD_I2C)
	i2c_out(DATA_CTL);
	for(int n=0; n<16; n++) {
		i2c_out(*text);
		//hold for busy flag
		text++;
	}
	i2c_stop();

	// add 30us delay?
}

void wait_busy() {
	/*
	i2c:
		i2c addr WR
		read busy flag cmd
	i2c:
		i2c addr RD
		loop:
			read data
			check busy flag
	*/
}

void lcd_show(unsigned char *text)	// 16 chars
{
	send_cmd(CLR_DISP_CMD);	// clear display command - also sets DDRAM address to 0
	// hold for busy flag
	// set ddram address? (or just set to home - this unnecessary since not shifting)
	// hold for busy flag
	send_data(text);	// write data to DDRAM
}

void init_lcd()
{
	// wait 40ms
	i2c_begin_tx();

	i2c_out(FUNC_SET_CMD | I2C_MODE | TWO_LN_MODE);
	// 26.3us delay
	i2c_out(FUNC_SET_CMD | I2C_MODE | TWO_LN_MODE);
	// 26.3us delay
	i2c_out(INT_OSC_CMD);
	// 26.3us delay
	i2c_out(CTRST_CMD);
	// 26.3us delay
	i2c_out(PWR_ICON_CMD | BOOSTER_ON);
	// 26.3us delay
	i2c_out(FOLLOW_CMD | FOLLOW_ON);
	// 200ms delay
	i2c_out(DISP_CTL_CMD | DISP_ON | CURS_ON | CURS_BLINK_ON);
	// 26.3us delay
	i2c_out(CLR_DISP_CMD);
	// 26.3us delay
	i2c_out(ENTRY_MODE_CMD | CURS_LR);
	// 26.3us delay

	i2c_stop();
}

void lcd_display(int r, int c, char *str) {
    // TODO: complete
}
