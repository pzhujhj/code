PROJS = server client

all: $(PROJS)

CC = gcc
CFLAGS = -Wall -o -g

server: server.o debug.o
	#$(CC) $(CFLAGS) $< -o $@
	$(CC) $(CFLAGS) $^ -o $@

client: client.o debug.o
	#$(CC) $(CFLAGS) $< -o $@
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(PROJS) *.o

