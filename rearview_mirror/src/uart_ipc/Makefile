objects = main.o uart.o ipc.o ipc_utils.o crc8.o
libs = -lpps
includes = -I.
libPath =

all: $(objects) 
	QCC -Vgcc_ntoarmv7le -o uart_ipc $(objects) $(libs) $(libPath)

main.o: main.cpp
	QCC -Vgcc_ntoarmv7le -c main.cpp $(includes)

uart.o: uart/uart.cpp uart/uart.h
	QCC -Vgcc_ntoarmv7le -c uart/uart.cpp $(includes)

ipc.o: ipc/ipc.c ipc/ipc.h
	QCC -Vgcc_ntoarmv7le -c ipc/ipc.c $(includes)

crc8.o: ipc/crc8.c ipc/crc8.h
	QCC -Vgcc_ntoarmv7le -c ipc/crc8.c $(includes)

ipc_utils.o: ipc/ipc_utils.c ipc/ipc_utils.h
	QCC -Vgcc_ntoarmv7le -c ipc/ipc_utils.c $(includes)


clean:
	rm uart_ipc $(objects)
