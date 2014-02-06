# these next two might not work under all OSes

ARCH = $(shell arch)
SYS  = $(shell uname)

include ../make/Make.$(SYS)
include ../make/Make.$(MACHINE)

obj/%.o:    %.cc
	$(CC) $(CFLAGS) -Iinclude -I/usr/include/ -c $< -o $@

MCL_LIB_ROOTNAME = MCL2core

all: dh

mclLib:	
	(cd .. ; make install)

dh: obj/direct_hit.o obj/biffs.o mclLib
	$(LD) -o dh obj/direct_hit.o obj/biffs.o -L.. -l$(MCL_LIB_ROOTNAME) -lm	

clean:
	-$(RM) *~
	-$(RM) *.o
	-$(RM) include/*~
