DEVICE	= atmega64
CLOCK	= 16000000
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

rfid: $(OBJ)
	avr-gcc -g -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) $^ -o $@

%.o: %.c
	avr-gcc -g -Wall -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -c $^ -o $@

clean:
	rm -rf rfid $(OBJ)

debug:
	avarice -d -j /dev/ttyUSB0 -R :3333
gdb:
	avr-gdb rfid -ex "target remote localhost:3333"
