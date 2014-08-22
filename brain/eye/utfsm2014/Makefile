CPP=g++
#CFLAGS=-O3 -Wall -DDEBUG # -DUNIT_TEST
CFLAGS=-g -Wall -DDEBUG
LIBS=-lopencv_core -lopencv_imgproc -lv3 -lm -lnavig -lautom -lpthread -ltimer -lds
DEF=

SRC= \
	eye.cc \
	yuyvrgb24.c \
	debug.cc \
	utfsm14.cc

_OBJ:=$(SRC:.c=.o) # Eager evaluation
OBJ=$(_OBJ:.cc=.o)

all:
	@echo "eye: unit test for eye.cc"

%.o: %.c
	$(CPP) $? -c $(CFLAGS) -o $@

%.o: %.cc
	$(CPP) $? -c $(CFLAGS) -o $@

test: $(OBJ) test.o
	$(CPP) $(OBJ) test.o -o test $(LIBS)

calib: $(OBJ) calib.o
	$(CPP) $(OBJ) calib.o -o calib $(LIBS)

stage1: $(OBJ) stage1.o
	$(CPP) $(OBJ) stage1.o -o stage1 $(LIBS)

stage2: $(OBJ) stage2.o
	$(CPP) $(OBJ) stage2.o -o stage2 $(LIBS)

stage3: $(OBJ) stage3.o
	$(CPP) $(OBJ) stage3.o -o stage3 $(LIBS)

graycheck: $(OBJ) graycheck.o
	$(CPP) $(OBJ) graycheck.o -o graycheck $(LIBS)

clean:
	rm -f *.o contest test calib stage1 stage2 stage3 graycheck
