all: .rvm .uart_ipc

.rvm:
	@make -C rvm CC=$(CC) CFLAG=$(CFLAG) ROOT_DIR=$(ROOT_DIR)
	
.uart_ipc:
	@make -C uart_ipc CC=$(CC) CFLAG=$(CFLAG) ROOT_DIR=$(ROOT_DIR)

.PHONY: clean
clean:
	@make -C rvm clean
	@make -C uart_ipc clean
