DEVICE	= atmega64
CLOCK	= 16000000
OBJECTS = rfid.o

rfid: rfid.c
	avr-gcc -mrelax -g -Wall -O3 -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) $^ -o $@

clean:
	rm rfid

debug:
	avarice -d -j /dev/ttyUSB0 -R :3333
gdb:
	avr-gdb rfid -ex "target remote localhost:3333"
