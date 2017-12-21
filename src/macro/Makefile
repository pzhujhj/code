PROJS = test

all: $(PROJS)

CC = gcc
CFLAGS = -Wall -o -g

#DEFS = -DTEST
#DEFS = -DTEST=0
#CFLAGS += $(DEFS)

test: ifdef.o
	$(CC) $(CFLAGS) $< -o $@
	#$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(PROJS) *.o

