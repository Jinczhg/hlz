#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <pthread.h>
#include <stdio.h>

#include "uart/uart.h"

#define PPS_RVM_FILE_PATH  "/pps/hinge-tech/rearview_mirror"

#define UART_DEV_FILE "/dev/ser2"

#define CHANNEL_OFFSET 0
#define TYPE_OFFSET    2

#define TURN_CHANNEL          0x01
#define REVERSING_CHANNEL     0x02
#define MANUAL_VISION_CHANNEL 0x04

#define TURN_LEFT_TYPE  0x01
#define TURN_RIGHT_TYPE 0x02

#define REVERSING_TYPE  0x01

#define MANUAL_VISION_LEFT 0x01
#define MANUAL_VISION_RIGHT 0x02

int s_turn_left = 0;
int s_turn_right = 0;
int s_reversing = 0;
char cmd[512] = {0};
uint8_t control = 0;

int pps_fd = -1;

void parse_data(uint8_t *data, int len)
{
    uint8_t channel;
    uint8_t type;
    uint8_t value;
    
    channel = data[CHANNEL_OFFSET];
    type = data[TYPE_OFFSET];
    value = data[TYPE_OFFSET + 1];

    if (channel == TURN_CHANNEL && s_reversing == 0)
    {
	if (type == TURN_LEFT_TYPE)
	{
	    s_turn_left = value;
	    if (s_turn_left == 1)
	    {
		s_turn_right = 0;
	    }
	    control = value ? 1 : 0; 
	}
	else if (type == TURN_RIGHT_TYPE)
	{
	    s_turn_right = value;
	    if (s_turn_right)
	    {
		s_turn_left = 0;
	    }
	    control = value ? 2 : 0;
	}
	else
	{
	    return;
	}

	sprintf(cmd, "car_control::%d", control);
    }
    else if (channel == REVERSING_CHANNEL)
    {
	if (type == REVERSING_TYPE)
	{
	    s_reversing = value;
	    control = s_reversing ? 3 : ((s_turn_left == 1) ? 1 : (s_turn_right == 1) ? 2 : 0);
	}
	else
	{
	    return;
	}

	sprintf(cmd, "car_control::%d", control);
    }
    else if (channel == MANUAL_VISION_CHANNEL)
    {
        if (type == MANUAL_VISION_LEFT && value == 1)
	{
	    sprintf(cmd, "car_vision::0");
	}
	else if(type == MANUAL_VISION_RIGHT && value == 1) 
	{
            sprintf(cmd, "car_vision::1");
	}
	else
	{
	    return;
	}
    }

    write(pps_fd, cmd, strlen(cmd));
    printf("cmd:%s\n", cmd);
}

int main(int argc, char** argv)
{
    pthread_t readpthr;
    int ret;

    pps_fd = open(PPS_RVM_FILE_PATH, O_WRONLY | O_CREAT);
    if (pps_fd < 0)
    {
	printf("open %s error: %s\n", PPS_RVM_FILE_PATH, strerror(errno));
	return 0;
    }

    if (argc > 1)
    {
        uart_init(argv[argc - 1], parse_data);
    }
    else
    {
        uart_init(UART_DEV_FILE, parse_data);
    }

    while (1)
    {
        sleep(10);
    }

    uart_exit();

    close(pps_fd);

    return 0;
}
