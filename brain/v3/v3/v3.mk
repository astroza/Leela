CC=gcc
#CFLAGS=-Wall -DDEBUG
CFLAGS=-Wall
V3_DIR?=
VPATH:=$(V3_DIR)

all: v3.o v3_monitor.o

install: all
	$(CC) $(V3_DIR)v3.o -shared -o  $(INSTALL_DIR)/lib/libv3.so
	$(CC) $(V3_DIR)v3_monitor.o  $(INSTALL_DIR)/lib/libv3.so -o $(INSTALL_DIR)/bin/v3_monitor
	cp $(V3_DIR)v3.h $(INSTALL_DIR)/include
	mkdir -p $(INSTALL_DIR)/lib/v3
	cp $(V3_DIR)init.sh $(INSTALL_DIR)/lib/v3

%.o: $(V3_DIR)%.c
	$(CC) -c $(CFLAGS) $< -o $(V3_DIR)$@

clean:
	rm -f $(V3_DIR)*.o
