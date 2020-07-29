CC=gcc
CFLAGS=-Wall -g -pipe -lwiringPi -lpthread

main: 
	$(CC) $(CFLAGS) -o bomba bomba.c ssd1306_i2c.c


clean: rm -f *.o
install: 
	install -m 755 bomba /usr/bin/bomba
	install -m 755 bomba.service /etc/systemd/system/bomba.service
