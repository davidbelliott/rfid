DEVICE	= atmega64a
CLOCK	= 16000000
SRC = $(wildcard *.c)
SRC_ASM = $(wildcard *.s)
C_OBJ = $(SRC:.c=.c.o)
ASM_OBJ = $(SRC_ASM:.s=.s.o)
OBJ = $(ASM_OBJ) $(C_OBJ)

rfid: $(OBJ)
	avr-gcc -g -O3 -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) $^ -o $@

%.c.o: %.c
	avr-gcc -g -O3 -Wall -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -c $^ -o $@

%.s.o: %.s
	avr-as -g $^ -o $@

clean:
	rm -rf rfid $(OBJ)

debug:
	avarice -d -j /dev/ttyUSB0 -R :3333
gdb:
	avr-gdb rfid -ex "target remote localhost:3333"
