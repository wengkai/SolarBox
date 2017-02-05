CC = gcc
CFLAGS = -Wall -std=c99

OBJS = test.o log.o log_stdout.o

test: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm *.o test
