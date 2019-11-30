DEVICE	= atmega64
CLOCK	= 16000000
OBJECTS = rfid.o

rfid: rfid.c
	avr-gcc -Wall -O3 -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) $^ -o $@

clean:
	rm rfid
