#ifndef LCD_H
#define LCD_H

#include "TWI_driver.h"


#define LCD_I2C		0b0111110	// slave address
#define I2C_RD		1
#define	I2C_WR		0

#define	CMD_CTL		0b00000000
					//0------- only one ctl byte per transmission
					//-0------ reg sel = 0, sending commmand
					//--000000 ctl byte

#define DATA_CTL	0b01000000
					//0------- only one ctl byte per transmission
					//-1------ reg sel = 1, sending data
					//--000000 ctl byte

#define CLR_DISP_CMD	0b00000000

#define	RTN_HOME_CMD	0b00000010

#define	ENTRY_MODE_CMD	0b00000100
#define	CURS_LR			0b00000010	// cursor moves L->R

#define DISP_CTL_CMD	0b00001000
#define DISP_ON			0b00000100
#define	CURS_ON			0b00000010
#define	CURS_BLINK_ON	0b00000001

#define FUNC_SET_CMD	0b00100000
#define	I2C_MODE		0b00010000
#define	TWO_LN_MODE		0b00001000
#define	DBL_HI_MODE		0b00000100	// double height disp, incompatible with two-line mode
#define	EXT_INST_MODE	0b00000001	// extended instruction set

#define INT_OSC_CMD		0b00010100

#define CTRST_CMD		0b01110000	// *may need to set last four bits (C3..0) for ref voltage V0
#define CTRST_HI		0b00001000 	// *from example

#define	PWR_ICON_CMD	0b01010000	// *may need to set last 2 bits (C5..4)
#define	ICON_MODE		0b00001000	// icon display on
#define BOOSTER_MODE	0b00000100	// booster mode on
#define CTRST_LO		0b00000010	// *from example

#define	FOLLOW_CMD		0b01100000	// *may need to set last 3 bits (Rab2..0) for V0 generator
#define	FOLLOW_ON		0b00001000	// follower circuit on
#define V0_AMP_RATIO	0b00000010	// *from example

#define	DDR_ADDR_CMD	0b10000000	// set DDRAM address

int init_lcd();
void lcd_show(unsigned char *text);
void lcd_display(int r, int c, char *str);
#endif // LCD_H
