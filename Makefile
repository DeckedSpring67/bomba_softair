CC=gcc
CFLAGS=-Wall -g -pipe -lwiringPi -lpthread

main: 
	$(CC) $(CFLAGS) -o bomba bomba.c ssd1306_i2c.c


clean: rm -f *.o
install: install -m644 bomba /usr/bin/bomba
