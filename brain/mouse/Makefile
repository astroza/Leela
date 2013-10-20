CC=gcc

all: mouse.o
	mkdir -p build
	$(CC) mouse.o -shared -o build/mouse.so

mouse.o: mouse.c
	$(CC) mouse.c -c
test: mouse.c
	$(CC) mouse.c -DDEBUG -DUNIT_TEST -o test -lpthread

install:
	cp build/mouse.so /usr/lib/libmouse.so
	cp mouse.h /usr/include
clean:
	rm -fr build
	rm -f *.o test
