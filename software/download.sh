#!/bin/sh
avrdude -v -p m64 -P /dev/ttyUSB0 -c jtag1 -U flash:w:blink
