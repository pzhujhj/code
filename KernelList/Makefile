SUBDIRS	:= bin

SUBDIRS += libs

APPS	:= KernelList

OBJS	:= unitil.o
OBJS	+= list_data.o
OBJS	+= checkApi.o
OBJS	+= main.o

#OBJS	+= init.o
#OBJS    += snmp.o

CROSS_COMPILE := gcc
ARM_CROSS_COMPILE := gcc
ARM_CROSS_COMPILE_AR := ar

AR	:= $(ARM_CROSS_COMPILE_AR)
CC	:= $(ARM_CROSS_COMPILE)

#INCLUDE	:= $(PWD) 
#INCLUDE +=-I/usr/include/

CFLAGS  += -Wall -O2

#LDFLAGS	:= -lpthread -lm 
LDFLAGS	:= -lpthread 

all: $(OBJS) 
	$(CC)  $(OBJS) $(BUILDLIBS) $(BUILDAGENTLIBS) -o $(APPS) $(LDFLAGS)

install:
	cp $(PWD)/ptp $(PWD)/bin

clean:
	rm -f $(APPS) $(OBJS)
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 
