CPP=g++
CFLAGS=-O3 -Wall -DDEBUG # -DUNIT_TEST
#CFLAGS=-g -Wall -DDEBUG
LIBS=-lopencv_core -lopencv_imgproc -lv3 -lm -lnavig -lautom -lpthread -ltimer
DEF=

SRC= \
	eye.cc \
	yuyvrgb24.c \
	debug.cc \
	aie.cc

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

contest: $(OBJ) contest.o
	$(CPP) $(OBJ) contest.o -o contest $(LIBS)

graycheck: $(OBJ) graycheck.o
	$(CPP) $(OBJ) graycheck.o -o graycheck $(LIBS)

clean:
	rm -f *.o contest test
