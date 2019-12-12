#include "lcd.h"
#include <util/delay.h>

// https://github.com/tomozh/arduino_ST7032


int send_cmd(unsigned char cmd) {

	tx_type *tx_frame;
	unsigned char temp[2];
	char state = 'n';
		
	tx_frame->slave_adr = LCD_I2C+W;		//Slave adr + Write
	tx_frame->size = 2;					//Number of bytes to send
	tx_frame->data_ptr = temp;			//Set the pointer to temp array	0
	tx_frame->data_ptr[0] = CMD_CTL;		
	tx_frame->data_ptr[1] = cmd;	

	state = Send_to_TWI(tx_frame);			//Call the Master TWI driver with
											//a pointer to the first struct 
											//in the package
											
	return state;							//If error occured during the TWI
											//comunication, return TWSR
											//If  no error, return SUCCESS		

	// add 30us delay?
}

int send_data(unsigned char *text) {

	tx_type *tx_frame;
	unsigned char temp[17];
	char state = 'n';
		
	tx_frame->slave_adr = LCD_I2C+W;		//Slave adr + Write
	tx_frame->size = 17;					//Number of bytes to send
	tx_frame->data_ptr = temp;			//Set the pointer to temp array	0
	tx_frame->data_ptr[0] = DATA_CTL;

	for(int i=0; i<16; i++)
		tx_frame->data_ptr[i+1] = text[i];	

	state = Send_to_TWI(tx_frame);			//Call the Master TWI driver with
											//a pointer to the first struct 
											//in the package
											
	return state;							//If error occured during the TWI
											//comunication, return TWSR
											//If  no error, return SUCCESS

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

int init_lcd()
{
	Init_TWI();			// set baud rate and enable TWI

	// wait 40ms
	_delay_ms(40);

	char state = 'n';
	tx_type *tx_frame;
	
	unsigned char temp[9] = {
		FUNC_SET_CMD | I2C_MODE | TWO_LN_MODE,
		FUNC_SET_CMD | I2C_MODE | TWO_LN_MODE,
		INT_OSC_CMD,
		CTRST_CMD,
		PWR_ICON_CMD | BOOSTER_MODE,
		FOLLOW_CMD | FOLLOW_ON,
		DISP_CTL_CMD | DISP_ON | CURS_ON | CURS_BLINK_ON,
		CLR_DISP_CMD,
		ENTRY_MODE_CMD | CURS_LR
	};
		
	tx_frame->slave_adr = LCD_I2C+W;		//Slave adr + Write
	tx_frame->size = 9;					//Number of bytes to send
	tx_frame->data_ptr = temp;			//Set the pointer to temp array	0

	state = Send_to_TWI(tx_frame);			//Call the Master TWI driver with
											//a pointer to the first struct 
											//in the package
											
	return state;							//If error occured during the TWI
											//comunication, return TWSR
											//If  no error, return SUCCESS
}

void lcd_display(int r, int c, char *str) {
    // TODO: complete
}


/*
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
*/
