CC = qcc
CFLAGS += -Vgcc_ntoarmv7le

all:
	@make -C src CC=$(CC) CFLAG=$(CFLAGS) ROOT_DIR=`pwd`

.PHONY: clean
clean:
	@make -C src clean
