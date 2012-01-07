# This makefile will build lbltp using the Cygwin tools on Windows.

# This makefile is normally parellel to the source for lbltp, use
# a VPATH directive to find it.
VPATH=../Source

CFLAGS=-I/usr/include -Wall -Wno-switch -Winline -Wmissing-prototypes -Wsign-compare -Wundef -Wstrict-prototypes -Wpacked -Wpointer-arith -Wmissing-declarations -Wno-char-subscripts -Wno-strict-aliasing -g -O3

LDFLAGS=

objs=fsrtns.o  functions.o  linemode.o  machine.o  vars.o

all : lbltp

lbltp : $(objs)
	${CC} $(LDFLAGS) -o $@ $(objs)

fsrtns.o : lbltp.h vars.h translate.h fsrtns.h machine.h functions.h
functions.o : lbltp.h vars.h translate.h functions.h fsrtns.h linemode.h machine.h
linemode.o : lbltp.h vars.h translate.h linemode.h functions.h machine.h
machine.o : lbltp.h vars.h translate.h machine.h linemode.h
vars.o : lbltp.h vars.h translate.h
