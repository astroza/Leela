CPP=g++
CFLAGS=-Wall -DDEBUG -g

all: automata.o

install: all
	$(CPP) automata.o -shared -o /usr/lib/libautom.so
	cp automata.h /usr/include

%.o: %.cc
	$(CPP) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o
