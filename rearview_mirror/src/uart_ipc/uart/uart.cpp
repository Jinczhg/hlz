#include "uart.h"
#include "ipc/ipc.h"
#include "ipc/ipc_utils.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <sys/time.h>

#include "uart.h"

class Data
{
public:
    explicit Data(const uint8_t *data,  int size)
    {
        mData = new uint8_t[size];
        memcpy(mData, data, size);
        mSize = size;
    }

    ~Data()
    {
        delete mData;
    }

    uint8_t *mData;
    int mSize;
};

static int s_uart_fd = -1;
static int s_exit = 0;
static receive_handle s_handle = NULL;

std::vector<class Data*> sData(0);
static pthread_t send_tid = -1;
static pthread_t read_tid = -1;

static pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t send_cond = PTHREAD_COND_INITIALIZER;

void* uart_send_thread_func(void* arg)
{
    class Data *data;
    int ret = 0;

    while (!s_exit)
    {
        while (sData.size() > 0)
        {
            data = sData[0];
            ret = write(s_uart_fd, data->mData, data->mSize);

            if (ret < 0)
            {
                printf("%s:%d write error: %s\n", __FUNCTION__, __LINE__, strerror(errno));
            }

            printf("uart_send data:");
            for (int i = 0; i < data->mSize; i++)
            {
                printf("%02X", data->mData[i]);
            }
            printf("\n\n");
            
            delete data;
            sData.erase(sData.begin());
            usleep(10000);
        }

        pthread_mutex_lock(&send_mutex);
        pthread_cond_wait(&send_cond, &send_mutex);
        pthread_mutex_unlock(&send_mutex);
    }
}

int uart_init(const char* uart_dev, receive_handle handle)
{
    s_uart_fd = open(uart_dev, O_RDWR | O_NOCTTY);
    if (s_uart_fd < 0)
    {
        printf("open %s error: %s\n", uart_dev, strerror(errno));
        return -1;
    }
    
    s_handle = handle;

    pthread_create(&send_tid, NULL, &uart_send_thread_func, NULL);
    pthread_create(&read_tid, NULL, &uart_read_thread_func, NULL);

    return 0;
}

void uart_exit()
{
    s_exit = 1;
    
    pthread_mutex_lock(&send_mutex);
    pthread_cond_signal(&send_cond);
    pthread_mutex_unlock(&send_mutex);

    if (s_uart_fd)
    {
        close(s_uart_fd);
    }
    
    pthread_join(send_tid, NULL);
    pthread_join(read_tid, NULL);
}

void* uart_read_thread_func(void*)
{
    struct ipc_data *ipc_de = 0;

    int size = 0;
    uint8_t buf[READ_BUF_MAX_SIZE];

    memset(buf, 0, READ_BUF_MAX_SIZE);

    while (1)
    {
        memset(buf, 0, READ_BUF_MAX_SIZE);
        size = read(s_uart_fd, buf, READ_BUF_MAX_SIZE);
        if(size < 0)
        {
            printf("%s:%d read error: %s\n", __FUNCTION__, __LINE__, strerror(errno));
            break;
        }

        printf("UART Receive\nlen: %d\n", size);
        for (int i = 0; i < size; i++)
        {
            printf("%02X", buf[i] & 0xFF);
        }
        printf("\n");
        
	for (int i = 0; i < size; i++)
        {
            if ((ipc_de = decode(buf[i])) != 0)
	    {
		if (s_handle)
		{
		    s_handle(ipc_de->data, ipc_de->len);
		}
	    }
        }
    }

    return NULL;
}

int uart_send(const uint8_t *data, int len)
{
    class Data *d = NULL;
    try
    {
       d = new  Data(data, len);
    }
    catch(...)
    {
        return 0;
    }

    sData.push_back(d);

    if (sData.size() == 1)
    {
        pthread_mutex_lock(&send_mutex);
        pthread_cond_signal(&send_cond);
        pthread_mutex_unlock(&send_mutex);
    }

    return len;
}
