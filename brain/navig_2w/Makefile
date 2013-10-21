CPP=g++
CFLAGS=-O3

%.o: %.cc
	$(CPP) $? -c $(CFLAGS) -o $@

all: navig.o navig_script.o navig_script_interp.o

install: all
	cp navig.h /usr/include/
	cp navig_script.h /usr/include/
	$(CPP) navig.o navig_script.o -shared -o /usr/lib/libnavig.so
	$(CPP) navig_script_interp.o -o /usr/bin/navig_script -lnavig -lv4 -lautom -ltimer -lmouse -lpthread
clean:
	rm -f *.o
