CFLAGS += -Wall -Werror  -I.

all: ucitestd 

ucitestd: ucitest.o
	$(CC) $(LDFLAGS) -o $@ $^ -lm -luci

clean:
	rm -f $(OBJ) *.o *.a

