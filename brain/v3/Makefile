INSTALL_DIR=/usr
V3_DIR=v3/
include v3/v3.mk

test: test.c
	$(CC) $? -lv3 -o $@ -lpthread

analog_test: analog_test.c
	$(CC) $? -lv3 -o $@ -lpthread -lds -lm
