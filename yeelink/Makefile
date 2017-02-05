CC = gcc
CFLAGS = -Wall -std=c99

OBJS = main.o yeelink.o ../log/log.o ../log/log_stdout.o

test: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm $(OBJS) test
