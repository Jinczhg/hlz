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
#include <stdlib.h>

#include "trace.h"
#include "avb.h"
#include "dec.h"
#include "gpu.h"
#include "rvm.h"

#define PPS_RVM_FILE "/pps/hinge-tech/rearview_mirror?wait,delta"

static char const version_string[] = "1.0.0";
static char const copyright_string[]= "Copyrith (c) 2017 Hinge-tech co.,ltd";

static int s_exit = 0;
static int pps_fd = -1;
rvm_config g_config;

static int parse_config_file(rvm_config* config)
{
    char file[PATH_MAX + 1] = {0};
    char path[PATH_MAX + 1] = {0};
    int exe_fd = -1;
    int len = 0;
    FILE *fp = NULL;
    char line[128];
    char *str;
    int i = 0;
    int ret = 0;
    
    exe_fd = open("/proc/self/exefile", O_RDONLY);
    len = read(exe_fd, path,  PATH_MAX);
    close(exe_fd);
    path[len] = '\0';
    for (i = len; i > 0; i--)
    {
	if (path[i] != '/')
	{
	    path[i] = '\0';
	}
	else
	{
	    break;
	}
    }
    
    sprintf(file, "%s%s", path, "rvm.cfg");
    fp = fopen(file, "r");
    if (fp == NULL)
    {
        ERROR("Failed to open config file:%s", file);
	return -1;
    }

    while (fgets(line, 128, fp) != NULL)
    {
        if ((str = strstr(line, "channel_front=")) != NULL)
        {
            config->channel_front = atoi(str+strlen("channel_front=")) - 1;
            if (config->channel_front < 0 || config->channel_front >= CHANNEL_NUM_MAX)
            {
                ERROR("channel_front = %d, not support", config->channel_front + 1);
                ret = -1;
                goto err;
            }
            DEBUG("channel_front=%d", config->channel_front + 1);
        }
        else if ((str = strstr(line, "channel_rear=")) != NULL)
        {
            config->channel_rear = atoi(str+strlen("channel_rear=")) - 1;
            if (config->channel_rear < 0 || config->channel_rear >= CHANNEL_NUM_MAX)
            {
                ERROR("channel_rear = %d, not support", config->channel_rear + 1);
                ret = -1;
                goto err;
            }
            DEBUG("channel_rear=%d", config->channel_rear + 1);
        }
        else if ((str = strstr(line, "camera_count=")) != NULL)
        {
            config->camera_count = atoi(str+strlen("camera_count="));
            DEBUG("camera_count=%d", config->camera_count);
        }
	else if ((str = strstr(line, "screen_direction=")) != NULL)
	{
	    config->screen_direction = atoi(str+strlen("screen_direction="));
	    DEBUG("screen_direction=%d", config->screen_direction);
	}
	else if ((str = strstr(line, "car_side=")) != NULL)	    
	{
	    config->car_side = atoi(str+strlen("car_side="));
	    DEBUG("car_side=%d", config->car_side);
	}
        else if ((str = strstr(line, "vision_move_step=")) != NULL)
	{
	    g_config.vision_move_step = atof(str+strlen("vision_move_step="));
	    DEBUG("vision_move_step=%f", g_config.vision_move_step);
	}
    }

err:
    if (fp)
    {
        fclose(fp);
    }
    
    return ret;
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
    int rc = 0;
    char *pps_str = NULL;
    int value = 0;
    const char *pps_cmd = "car_control::0\n";

    if(argc == 2)
    {
        if(strcmp(argv[1], "--version") == 0)
        {
            printf("rearview mirror demo version %s\n%s\n",
                version_string, copyright_string);
            exit(0);
        }
    }

    memset(&g_config, 0, sizeof(g_config));
    g_config.channel_front = 0;
    g_config.channel_rear = 1;
    g_config.camera_count = 2;
    g_config.car_side = CAR_SIDE_LEFT;
    g_config.car_control = CAR_MOVE_FORWARD;
    g_config.car_vision = CAR_MANUAL_VISION_NUM;

    if (parse_config_file(&g_config))
    {
        ERROR("parse_config_file fail");
        return -1;
    }

    rc = decode_init();
    if (rc)
    {
	goto deinit;
    }

    rc = gpu_init(&g_config);
    if (rc)
    {
	goto deinit;
    }

    rc = avb_init();
    if (rc)
    {
	goto deinit;
    }

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
    
    //sleep(1);
    //start capture
    avb_signal(1); 
  
    pps_fd = open(PPS_RVM_FILE, O_RDWR | O_CREAT, 0666);    
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
		memset(buf, 0, sizeof(buf));

		len = read(pps_fd, buf, sizeof(buf) - 1);
		if (len < 0)
		{
		    ERROR("pps read error: %s", strerror(errno));
		    continue;
		}

		DEBUG("pps read len = %d, content:\n%s", len, buf);

		if ((pps_str = strstr(buf, "car_control::")) != NULL)
		{
                    value = atoi(pps_str + strlen("car_control::"));
		    if (value >= 0 && value < CAR_CONTROL_NUM)
		    {
			g_config.car_control = (CAR_CONTROL)value;
		    }
		    else
		    {
			ERROR("car control not support value %d", value);
		    }
		}
		else if ((pps_str = strstr(buf, "car_vision::")) != NULL)
		{
                    value = atoi(pps_str + strlen("car_vision::"));
		    if (value >= 0 && value < CAR_MANUAL_VISION_NUM)
		    {
			g_config.car_vision = (CAR_MANUAL_VISION)value;
		    }
		    else
		    {
			ERROR("car vision not support value %d", value);
		    }
		}
	    }
	}
    }

    close(pps_fd);

    DEBUG("to end the threads");
    
    // threads exit
    //stop capture
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

deinit:
    //deinit
    avb_deinit();
    decode_deinit();
    gpu_deinit();

    DEBUG("%s exit......", argv[0]);

    return 0;
}
