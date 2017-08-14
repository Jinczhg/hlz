#include "ipc.h"
#include "uart/uart.h"
#include "ipc_utils.h"

int ipc_recv(unsigned char* data, int len)
{
	return 0;
}

int ipc_send(unsigned char* data, int len)
{
	struct ipc_data* en_data = encode(data, len);
	return uart_send(en_data->data, en_data->len);
}
