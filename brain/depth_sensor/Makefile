CC=gcc

all: depth_sensor.o
	mkdir -p build
	$(CC) -lm -shared depth_sensor.o -o build/depth_sensor.so

depth_sensor.o:
	$(CC) depth_sensor.c -c

test:
	$(CC) depth_sensor.c -DDEBUG -DUNIT_TEST -lm -o test

install:
	cp build/depth_sensor.so /usr/lib/libds.so
	cp depth_sensor.h /usr/include

clean:
	rm -fr build
	rm *.o
