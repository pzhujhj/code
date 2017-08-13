PROJS = 1 binary my_strstr

all: $(PROJS)

CC = gcc
CFLAGS = -Wall -o -g

1: 1.o
	$(CC) $(CFLAGS) $< -o $@
	#$(CC) $(CFLAGS) $^ -o $@

binary: binary.o
	$(CC) $(CFLAGS) $< -o $@
	#$(CC) $(CFLAGS) $^ -o $@

my_strstr: my_strstr.o
	$(CC) $(CFLAGS) $< -o $@
	#$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(PROJS) *.o

