CFLAGS += -Wall -Werror  -I.
ifdef HAVE_GPS_LED
	CFLAGS += -DHAVE_GPS_LED
endif

all: ucitestd 

ucitestd: ucitest.o
	$(CC) $(LDFLAGS) -o $@ $^ -lm -luci

clean:
	rm -f $(OBJ) *.o *.a

