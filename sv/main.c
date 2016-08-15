/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-06-01
 * Author: ryan
 */

#include <pthread.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "trace.h"
#include "avb.h"
#include "dec.h"
#include "gpu.h"

#ifdef CALC_ALGO_TIME
#include "calc.h"
#endif

#define PPS_CAMERA_FILE "/pps/hinge-tech/camera"

static int s_exit = 0;
static int pps_fd = -1;

void signal_fun(int signo)
{
    s_exit = 1;
    
    if (pps_fd != -1)
    {
	close(pps_fd);
    }

    DEBUG("all exit....");
}

int main(int argc, char **argv)
{
    pthread_t pid_avb_recv = 0, pid_avb_proc;
    pthread_t pid_dec[CHANNEL_NUM_MAX*2] = {0};
    pthread_t pid_gpu = 0;

    fd_set rfd;
    char buf[256] = {0};
    int len = 0;
    int i = 0;
    int capture = 0;
    const char *pps_cmd = "capture::stop\n";

    signal(SIGINT, signal_fun);

#ifdef CALC_ALGO_TIME
    calc_init();
#endif

    decode_init();
    gpu_init();
    avb_init();

    // gpu/display thread
    pthread_create(&pid_gpu, NULL, gpu_thread, NULL);

    // decode threads
    for (i = 0; i < CHANNEL_NUM_MAX; i++)
    {
        pthread_create(&pid_dec[i], NULL, decode_thread, &i);
    }

    // avb thread
    pthread_create(&pid_avb_proc, NULL, avb_proc_thread, NULL);
    pthread_create(&pid_avb_recv, NULL, avb_recv_thread, NULL);


    //prio
    if (pthread_setschedprio(pid_gpu, 63) != EOK)
    {
	ERROR("gpu thread: pthread_setschedprio() failed");
    }

    for (i = 0; i < CHANNEL_NUM_MAX; i++)
    {
	if (pthread_setschedprio(pid_dec[i], 63) != EOK)
	{
	    ERROR("dec thread: pthread_setschedprio() failed");
	}
    }

    if (pthread_setschedprio(pid_avb_proc, 63) != EOK)
    {
	ERROR("avb proc thread: pthread_setschedprio() failed");
    }
    
    if (pthread_setschedprio(pid_avb_recv, 63) != EOK)
    {
	ERROR("avb recv thread: pthread_setschedprio() failed");
    }
  
/*
    if (pthread_setschedprio(pthread_self(), 25) != EOK)
    {
	ERROR("main thread: pthread_setschedprio() failed");
    }
*/
    pps_fd = open(PPS_CAMERA_FILE, O_RDWR | O_CREAT, 0666);    
    if (pps_fd < 0)
    {
	ERROR("open pps file failed");
    }

    write(pps_fd, pps_cmd, strlen(pps_cmd));

    while (!s_exit)
    {
	FD_ZERO(&rfd);
	FD_SET(pps_fd, &rfd);
	
	if (select(1 + pps_fd, &rfd, 0, 0, NULL) > 0)
	{
	    if (FD_ISSET(pps_fd, &rfd))
	    {
		len = read(pps_fd, buf, sizeof(buf) - 1);
		if (len < 0)
		{
		    ERROR("pps read error: %s", strerror(errno));
		    continue;
		}

		DEBUG("pps read len = %d, content:\n%s", len, buf);

		if (strstr(buf, "capture::start") != NULL)
		{
		    capture = 1;
		}
		else
		{
		    capture = 0;
		}

#ifdef CALC_ALGO_TIME
		if (strstr(buf, "calc_flush::1") != NULL)
		{
		    write(pps_fd, "calc_flush::0\n", strlen("calc_flush::0\n"));
		    avb_signal(0);
                    calc_flush_log();
		}
#endif
		avb_signal(capture);
		gpu_visuable(capture);
	    }
	}
    }

    DEBUG("to end the threads");
    // threads exit
    avb_signal(0);
    //avb_exit();
    for (i = 0; i < CHANNEL_NUM_MAX; i++)
    {
	decode_exit(i);
    }
    gpu_exit();

    //join
    pthread_join(pid_avb_recv, NULL);
    pthread_join(pid_avb_proc, NULL);
    for (i = 0; i < CHANNEL_NUM_MAX; i++)
    {
	pthread_join(pid_dec[i], NULL);
    }
    pthread_join(pid_gpu, NULL);

    //deint
    avb_deinit();
    decode_deinit();
    gpu_deinit();

    DEBUG("%s exit......", argv[0]);

    return 0;
}
