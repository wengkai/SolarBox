CC = gcc
CFLAGS = -Wall -std=c99 -lwiringPi

OBJS = test.o tlc1543.o

test: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm $(OBJS) test
