TARGETS = ppmcvt
OBJS = ppmcvt.o pmb_aux.o
CFLAGS=-g
# LDLIBS=-lm
CC=gcc

all: $(TARGETS)

time: ppmcvt.o pmb_aux.o
	gcc -o ppmcvt ppmcvt.c pbm_aux.c  pbm.c

clean:
	$(RM) -f $(TARGETS) $(OBJS)
