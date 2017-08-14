#ifndef UART_H
#define UART_H

#include <stdint.h>

#define READ_BUF_MAX_SIZE 32
#define READ_BYTE_COUNT 1
#define COMMAND_BYTE_COUNT 7

typedef void (*receive_handle)(uint8_t *data, int len);

int uart_init(const char* uart_dev, receive_handle handle);

void uart_exit();

void* uart_read_thread_func(void*);

void* uart_door_process_thread_func(void *);

void* uart_seat_process_thread_func(void *seat_fd);

void* uart_gesture_process_thread_func(void *);

int uart_send(const uint8_t *data, int len);

#endif
