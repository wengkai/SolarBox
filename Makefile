CC = gcc
CFLAGS = -Wall -std=c99 -lwiringPi -lpthread

OBJS = main.o log/log.o log/log_stdout.o \
	tlc1543/tlc1543.o httpd/httpd.o charger/charger.o \
	yeelink/yeelink.o

solar: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm $(OBJS) solar

