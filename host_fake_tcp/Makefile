# these next two might not work under all OSes

ARCH = $(shell arch)
SYS  = $(shell uname)

include ../make/Make.$(SYS)
include ../make/Make.$(MACHINE)

%.o:    %.cc
	$(CC) $(CFLAGS) -Iinclude -I/usr/local/include -I../api -c $< -o $@

MCL_LIB_ROOTNAME = MCL2core

all: tcphost

mclLib:	
	(cd .. ; make all)

tcphost: mclLib simHost.o
	$(LD) -o tcphost simHost.o -L/usr/local/lib -lm -lUMBC

clean:
	-$(RM) *~
	-$(RM) *.o
	-$(RM) tcphost
