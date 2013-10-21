CC=gcc
#CFLAGS=-Wall
CFLAGS=-Wall
INSTALL_DIR=/usr

all: build/v4.o build/v4_test.o build/v4_monitor.o

install: all
	$(CC) build/v4.o -shared -o  $(INSTALL_DIR)/lib/libv4.so
	$(CC) build/v4_test.o $(INSTALL_DIR)/lib/libv4.so -o $(INSTALL_DIR)/bin/v4_test
	$(CC) build/v4_monitor.o -o $(INSTALL_DIR)/bin/v4_monitor
	cp v4.h $(INSTALL_DIR)/include
	mkdir -p /usr/lib/v4
	cp init.sh /usr/lib/v4
	chmod +x /usr/lib/v4/init.sh
	cp v4_monitor.service /etc/systemd/system

build/%.o: %.c
	mkdir -p build
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f build/*.o
