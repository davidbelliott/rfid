#!/bin/sh
avrdude -v -p m64 -P /dev/ttyUSB0 -c jtag1 -U efuse:w:0xFF:m
avrdude -v -p m64 -P /dev/ttyUSB0 -c jtag1 -U hfuse:w:0x19:m
avrdude -v -p m64 -P /dev/ttyUSB0 -c jtag1 -U lfuse:w:0xEF:m
