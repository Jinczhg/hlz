/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 * 
 * Date: 2016-06-01
 * Author: ryan
 */

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

#include <sys/neutrino.h>
#include <screen/screen.h>

#include "gpu.h"
#include "dec.h"
#include "trace.h"
#include "sv.h"
#include "stitchalgo.h"

#ifdef CALC_ALGO_TIME
#include "calc.h"
#endif

#define GPU_BUFFER_NUM 4

static pthread_cond_t gpu_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gpu_mutex = PTHREAD_MUTEX_INITIALIZER;
static OutputBuffer *s_buffers[GPU_BUFFER_NUM];
static YUVBuffer s_front = {SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, 0 };
static YUVBuffer s_rear = {SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, 0 };
static YUVBuffer s_left = {SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, 0 };
static YUVBuffer s_right = {SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, 0 };
static YUVBuffer s_output = {SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, 0 };

static screen_window_t *s_screen_win_p = NULL;
static screen_buffer_t *s_screen_buf_p = NULL;
static int s_exit = 0;

/* preferred resolution list from RIM */
static struct _preferred_mode {
    int  width;
    int  height;
    int  refresh;
    int  interlaced;
} preferred_modes [] = {
    { 1920, 1080, 60, 0 },
    { 1920, 1080, 50, 0 },
    { 1280, 720, 60, 0 },
    { 1280, 720, 50, 0 },
    { 1920, 1080, 60, 1 },
    { 1920, 1080, 50, 1 },
    { 720, 480, 60, 0 },
    { 720, 576, 50, 0 },
    { 720, 480, 60, 0 },
    { 720, 576, 50, 0 },
    { 640, 480, 60, 0 },
};

int find_preferred_mode(screen_display_mode_t *modes, int modesSize)
{
    int i, j;
    int found_preferred = 0;
    int preferred_idx = 0;

    for (i=0; i < sizeof (preferred_modes); i++)
    {
	for (j=0; j < modesSize; j++) 
	{
	    if (modes[j].width == preferred_modes[i].width &&
		    modes[j].height == preferred_modes[i].height &&
		    modes[j].refresh == preferred_modes[i].refresh &&
		    modes[j].interlaced == preferred_modes[i].interlaced) 
	    {
		preferred_idx = j;
		found_preferred = 1;
		break;
	    }
	}

	if (found_preferred) 
	{
	    break;
	}
    }

    return preferred_idx;
}



int display_setup(screen_context_t screen_ctx, int idx, int verbose)
{
    int val;                       		
    int rval = -1;       		
    int rc;                        		
    int i;                         		
    screen_display_t *displays;
    int display_count = 0, port;
    int m = 0;

    rc = screen_get_context_property_iv(screen_ctx, SCREEN_PROPERTY_DISPLAY_COUNT, &display_count);

    if (idx > display_count) 
    {
	goto fail;
    }

    displays = (screen_display_t *)malloc(display_count * sizeof(screen_display_t));

    rc = screen_get_context_property_pv(screen_ctx, SCREEN_PROPERTY_DISPLAYS, (void **)displays);

    i = idx;
    if (verbose) 
    {
	char dev_id[100];
	rc = screen_get_display_property_cv(displays[i], SCREEN_PROPERTY_ID_STRING, sizeof(dev_id), dev_id);
	if ( rc != -1 ) 
	{
	    printf("display devid %s\n", dev_id );
	}
    }

    screen_get_display_property_iv(displays[i], SCREEN_PROPERTY_ATTACHED, &val);
    if (verbose)
    {
	screen_get_display_property_iv(displays[i], SCREEN_PROPERTY_ID, &port);
	INFO("display %d (port %d) is %stached\n", i, port, val?"at":"de");
    }
    if (val) 
    {
	screen_get_display_property_iv(displays[i], SCREEN_PROPERTY_MODE_COUNT, &val);
	if (verbose)
	{
	    INFO("\tmode count is %d\n", val);
	}
	screen_display_mode_t *modes = (screen_display_mode_t *)malloc(val * sizeof(*modes));
	rc = screen_get_display_modes(displays[i], val, modes);
	if (verbose) 
	{
	    INFO("modes is %p, rc is %d\n", modes, rc);
	    for (m = 0; m < val; m++) 
	    {
		INFO("%d: %dx%d%s @ %dHz %d:%d flags %#08x index %d\n", m, modes[m].width, modes[m].height, 
			modes[m].interlaced ? "i":"p", 
			modes[m].refresh,
			modes[m].aspect_ratio[0], modes[m].aspect_ratio[1], modes[m].flags, modes[m].index);
	    }
	}
		
	if (val > 1) 
	{
	    m = find_preferred_mode(modes, val);

	    rc = screen_set_display_property_iv(displays[i], SCREEN_PROPERTY_MODE, &m);
	    if (verbose)
	    {
		INFO("set size %dx%d, rc = %d\n", modes[m].width, modes[m].height, rc);
	    }
	    rc = screen_flush_context(screen_ctx, 0);
	    if (verbose)
	    {
		INFO("flushed, rc = %d\n", rc);
	    }
	}
    }

    rval = 0;

fail:
    return rval;
}

int gpu_init()
{
    int i = 0;
    int tw = decode_padded_width() * decode_padded_height() * 3 / 2;

    for (i = 0; i < GPU_BUFFER_NUM; i++)
    {
	s_buffers[i] = output_alloc(1, decode_padded_width(), decode_padded_height());

	if (!s_buffers[i])
	{
	    i--;
	    while (i != -1)
	    {
		output_free(s_buffers[i], 1);
		s_buffers[i] = NULL;
		i--;
	    }
	    return -1;
	}
	memset(s_buffers[i]->buf, 0, tw);
    }

    s_front.bufYUV = (uint32_t)(s_buffers[0]->buf);
    s_rear.bufYUV = (uint32_t)(s_buffers[1]->buf);
    s_left.bufYUV = (uint32_t)(s_buffers[2]->buf);
    s_right.bufYUV = (uint32_t)(s_buffers[3]->buf);

    DEBUG("init OK");
    return 0;
}

void gpu_deinit()
{
    int i = 0;

    for (i = 0; i < GPU_BUFFER_NUM; i++)
    {
	if (!s_buffers[i])
	{
	    free(s_buffers[i]);
	    s_buffers[i] = NULL;
	}
    }

    DEBUG("deinit OK");
}

OutputBuffer* gpu_get_buffer(int channel)
{
    return s_buffers[channel];
}

void* gpu_thread(void *arg)
{
    screen_context_t screen_ctx;
    screen_window_t screen_win;
    screen_buffer_t screen_buf[2];
    screen_event_t screen_ev;
    int size[2] = {SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT};
    int pos[2] = { 0, 0 };
    int spos[2] = { 0, 0 };
    int format = SCREEN_FORMAT_NV12;
    int val;
    const char *tok;
    int stride;
    void *pointer;
    int count = 1; //for images count, default 1
    int interval = 0; //0ms as default
    int skip = 0;//image switching skip
    int img_scale = 0;
    int rc, i = 0;
    int ndisplays;                                 /* number of displays */
    const char *display = "2";              /* the display to create our window on */
    screen_display_t screen_disp;                  /* native handle for our display */
    int verbose = 0;
    int init_display = 1;
    int mode_idx = -1;
    int pipeline = 1;
    int transp = SCREEN_TRANSPARENCY_NONE;
    int transp_set = 0;
    int zorder = 20;

    int dis = 0;

    int rect[4] = {0, 0, size[0], size[1]};

    int tw = SV_IMAGE_WIDTH * SV_IMAGE_HEIGHT * 3 / 2;

    int data = 0x01;
    //ThreadCtl(_NTO_TCTL_RUNMASK, &data);

    rc = screen_create_context(&screen_ctx,SCREEN_APPLICATION_CONTEXT);
    if (rc) 
    {
	ERROR("screen_context_create");
	goto fail1;
    }

    rc = screen_get_context_property_iv(screen_ctx, SCREEN_PROPERTY_DISPLAY_COUNT, &ndisplays);
    if (rc) 
    {
	ERROR("screen_get_context_property_iv(SCREEN_PROPERTY_DISPLAY_COUNT)");
	goto fail2;
    }

    rc = screen_create_window(&screen_win, screen_ctx);
    if (rc) 
    {
	ERROR("screen_create_window");
	goto fail2;
    }

    screen_display_t *displays;
    displays = calloc(ndisplays, sizeof(*displays));
    if (displays == NULL) 
    {
	ERROR("could not allocate memory for display list");
	goto fail3;
    }

    rc = screen_get_context_property_pv(screen_ctx, SCREEN_PROPERTY_DISPLAYS, (void **)displays);
    if (rc) 
    {
	ERROR("screen_get_context_property_pv(SCREEN_PROPERTY_DISPLAYS)");
	free(displays);
	goto fail3;
    }

    if (isdigit(*display)) 
    {
	int want_id = atoi(display);
	for (i = 0; i < ndisplays; ++i) 
	{
	    int actual_id = 0;  // invalid
	    (void)screen_get_display_property_iv(displays[i], SCREEN_PROPERTY_ID, &actual_id);
	    if (want_id == actual_id) 
	    {
		break;
	    }
	}
    } 
    else 
    {
	int type = -1;
	if (strcmp(display, "internal") == 0) 
	{
	    type = SCREEN_DISPLAY_TYPE_INTERNAL;
	} 
	else if (strcmp(display, "composite") == 0) 
	{
	    type = SCREEN_DISPLAY_TYPE_COMPOSITE;
	} 
	else if (strcmp(display, "svideo") == 0) 
	{
	    type = SCREEN_DISPLAY_TYPE_SVIDEO;
	} 
	else if (strcmp(display, "YPbPr") == 0) 
	{
	    type = SCREEN_DISPLAY_TYPE_COMPONENT_YPbPr;
	} 
	else if (strcmp(display, "rgb") == 0) 
	{
	    type = SCREEN_DISPLAY_TYPE_COMPONENT_RGB;
	} 
	else if (strcmp(display, "rgbhv") == 0) 
	{
	    type = SCREEN_DISPLAY_TYPE_COMPONENT_RGBHV;
	} 
	else if (strcmp(display, "dvi") == 0) 
	{
	    type = SCREEN_DISPLAY_TYPE_DVI;
	} 
	else if (strcmp(display, "hdmi") == 0) 
	{
	    type = SCREEN_DISPLAY_TYPE_HDMI;
	} 
	else if (strcmp(display, "other") == 0) 
	{
	    type = SCREEN_DISPLAY_TYPE_OTHER;
	} 
	else 
	{
	    ERROR("unknown display type %s\n", display);
	    free(displays);
	    goto fail3;
	}

	for (i = 0; i < ndisplays; i++) 
	{
	    screen_get_display_property_iv(displays[i], SCREEN_PROPERTY_TYPE, &val);
	    if (val == type) 
	    {
		break;
	    }
	}
    }

    if (i >= ndisplays) 
    {
	ERROR("couldn't find display %s\n", display);
	free(displays);
	goto fail3;
    }

    screen_disp = displays[i];
    free(displays);

    if (init_display)
    {
	if ( display_setup(screen_ctx,  i, verbose) == -1) 
	{
	    ERROR("display setup failed");
	    goto fail3;
	}
    } 
    else if (mode_idx >= 0) 
    {
	int mode_count = 0, rc;
	screen_display_mode_t *modes;
	rc = screen_get_display_property_iv(screen_disp, SCREEN_PROPERTY_MODE_COUNT, &mode_count);
	
	if (verbose) 
	{
	    INFO("get MODE_COUNT property, count = %d, rc = %d\n", mode_count, rc );
	}

	if (mode_idx >= mode_count) 
	{
	    ERROR("couldn't find mode %d, last mode is %d\n", mode_idx, mode_count);
	    goto fail3;
	}

	modes = malloc(mode_count * sizeof(modes[0]));
	if (!modes) 
	{
	    perror("display modes list malloc failed");
	    goto fail3;
	}

	rc = screen_get_display_modes(screen_disp, mode_count, modes);
	if (verbose) 
	{
	    INFO("get display modes, rc = %d\n", rc );
	}
	
	rc = screen_set_display_property_iv(screen_disp, SCREEN_PROPERTY_MODE, &mode_idx);
	if (verbose) 
	{
	    INFO("set size %dx%d, rc = %d\n", modes[mode_idx].width, modes[mode_idx].height, rc );
	}
	rc = screen_flush_context(screen_ctx, 0);
	if (verbose) 
	{
	    INFO("flushed, rc = %d\n", rc );
	}
    }

    rc = screen_set_window_property_pv(screen_win, SCREEN_PROPERTY_DISPLAY, (void **)&screen_disp);
    if (rc) 
    {
	ERROR("screen_set_window_property_pv(SCREEN_PROPERTY_DISPLAY)");
	goto fail3;
    }

    val = SCREEN_USAGE_WRITE;
    if (pipeline != -1) 
    {
	val |= SCREEN_USAGE_OVERLAY;
    }
    rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_USAGE, &val);
    if (rc) 
    {
	ERROR("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE)");
	goto fail3;
    }

    rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_FORMAT, &format);
    if (rc) 
    {
	ERROR("screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT)");
	goto fail3;
    }

    if (size[0] == -1 || size[1] == -1) 
    {
        rc = screen_get_display_property_iv(screen_disp, SCREEN_PROPERTY_SIZE, size);
	if (rc) 
	{
	    ERROR("screen_get_window_property_iv(SCREEN_PROPERTY_SIZE)");
	    goto fail3;
	}
    }

    if (transp_set) 
    {
	if (verbose) 
	{
	    INFO("transp = %d\n", transp);
	}
	
	rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_TRANSPARENCY, &transp);
	if (rc) 
	{
	    ERROR("screen_set_window_property_iv(SCREEN_PROPERTY_TRANSPARENCY)");
	    goto fail3;
	}
    } 
    else 
    {
	if (verbose) 
	{
	    INFO("transp = (not set)\n", transp);
	}
    }

    if (verbose) 
    {
        INFO("zorder = %d\n", zorder);
    }

    rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_ZORDER, &zorder);
    if (rc) 
    {
	ERROR("screen_set_window_property_iv(SCREEN_PROPERTY_ZORDER)");
	goto fail3;
    }

    if (pipeline != -1) 
    {
	if (verbose) 
	{
	    INFO("pipeline = %d\n", pipeline);
	}
	
	rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_PIPELINE, &pipeline);
	if (rc) 
	{
	    ERROR("screen_set_window_property_iv(SCREEN_PROPERTY_PIPELINE)");
	    goto fail3;
	}
    }

    rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE, size);
    if (rc) 
    {
	ERROR("screen_set_window_property_iv(SCREEN_PROPERTY_BUFFER_SIZE)");
	goto fail3;
    }

    rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SOURCE_SIZE, size);
    if (rc) 
    {
	ERROR("screen_set_window_property_iv(SCREEN_PROPERTY_SOURCE_SIZE)");
	goto fail3;
    }

    if ( spos[0] != 0 || spos[1] != 0 ) 
    {
	rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SOURCE_POSITION, spos);
	if (rc) 
	{
	    ERROR("screen_set_window_property_iv(SCREEN_PROPERTY_SOURCE_POSITION)");
	    goto fail3;
	}
    }

    rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SIZE, size);
    if (rc) 
    {
	ERROR("screen_set_window_property_iv(SCREEN_PROPERTY_SIZE)");
	goto fail3;
    }

    if (pos[0] != 0 || pos[1] != 0) 
    {
	rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_POSITION, pos);
	if (rc) 
	{
	    ERROR("screen_set_window_property_iv(SCREEN_PROPERTY_POSITION)");
	    goto fail3;
	}
    }

    // Set ID string for debugging via /dev/screen.
    {
	const char *idstr = "display_camera";
	screen_set_window_property_cv(screen_win,
			SCREEN_PROPERTY_ID_STRING, strlen(idstr), idstr);
    }

    rc = screen_create_window_buffers(screen_win, 1);
    if (rc) 
    {
	ERROR("screen_create_window_buffers");
	goto fail3;
    }

    rc = screen_get_window_property_pv(screen_win, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)screen_buf);
    if (rc) 
    {
	ERROR("screen_get_window_property_pv(SCREEN_PROPERTY_RENDER_BUFFERS)");
	goto fail3;
    }

    rc = screen_get_buffer_property_pv(*screen_buf, SCREEN_PROPERTY_POINTER, &pointer);
    if (rc)
    {
	ERROR("screen_get_buffer_property_pv(SCREEN_PROPERTY_POINTER)");
	goto fail3;
    }

    rc = screen_get_buffer_property_iv(*screen_buf, SCREEN_PROPERTY_STRIDE, &stride);
    if (rc) 
    {
	ERROR("screen_get_buffer_property_iv(SCREEN_PROPERTY_STRIDE)");
	goto fail3;
    }

    rc = screen_create_event(&screen_ev);
    if (rc) 
    {
	ERROR("screen_create_event");
	goto fail3;
    }

    s_output.bufYUV = (uint32_t)pointer;
    memset(pointer, 0, tw);
    s_screen_win_p = &screen_win;
    s_screen_buf_p = screen_buf;

    uint32_t in_stride = decode_padded_width(); 
    uint32_t y_offset = decode_y_offset();
    uint32_t uv_offset = decode_uv_offset();
    float diff = 0;

    initStitching(in_stride, y_offset, uv_offset, SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT);

    DEBUG("gpu thread is ready");

    while (!s_exit)
    {
	pthread_mutex_lock(&gpu_mutex);
	pthread_cond_wait(&gpu_cond, &gpu_mutex);     
	pthread_mutex_unlock(&gpu_mutex);

        //gettimeofday(&ttime, NULL);
	//DEBUG("cnt %d: b_display time sec:%d,usec:%d\n", s_cnt_c, ttime.tv_sec, ttime.tv_usec);
	if (s_exit)
	{
	    break;
	}

#ifdef CALC_ALGO_TIME
	calc_stitch_frame_start();
#endif

	doStitching(&s_front, &s_rear, &s_left, &s_right, &s_output);

#ifdef CALC_ALGO_TIME
	calc_stitch_frame_end();
#endif

#ifdef CALC_ALGO_TIME
	calc_disp_frame_start();
#endif
        rc = screen_post_window(screen_win, *screen_buf, 1, rect, 0);
	if (rc) 
	{
	    ERROR("screen_post_window() failed: img_errno: %d\n", rc);
	}

#ifdef CALC_ALGO_TIME
	calc_disp_frame_end();
#endif

#ifdef CALC_ALGO_TIME
	calc_disp_frame_info();
#endif
    }


fail4:
    screen_destroy_event(screen_ev);

fail3:
    screen_destroy_window(screen_win);

fail2:
    screen_destroy_context(screen_ctx);

fail1:

    DEBUG("gpu thread exit");
    return 0;
}

void gpu_signal()
{
    static int frame_cnt = 0;

    pthread_mutex_lock(&gpu_mutex);

#ifdef CALC_ALGO_TIME 

#ifdef CAMERA_CHANNEL
    calc_disp_frame_signal();
    pthread_cond_signal(&gpu_cond);
#else
    if (++frame_cnt == CHANNEL_NUM_MAX)
    {
	calc_disp_frame_signal();
	pthread_cond_signal(&gpu_cond);
	frame_cnt = 0;
    }
#endif //CAMERA_CHANNEL

#else

    if (++frame_cnt == CHANNEL_NUM_MAX)
    {
	calc_disp_frame_signal();
	pthread_cond_signal(&gpu_cond);
	frame_cnt = 0;
    }

#endif //CALC_ALGO_TIME

    pthread_mutex_unlock(&gpu_mutex);
}

void gpu_visuable(int visuable)
{
    int pos[2] = {0, 0};
    int rect[4] = {0, 0, SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT};
    if (visuable == 0)
    {
	pos[0] = -SV_IMAGE_WIDTH;
    }

    if (s_screen_win_p)
    {
	screen_set_window_property_iv(*s_screen_win_p, SCREEN_PROPERTY_POSITION, pos);
	screen_post_window(*s_screen_win_p, *s_screen_buf_p, 1, rect, 0);
    }
}

void gpu_exit()
{
    s_exit = 1;
    pthread_mutex_lock(&gpu_mutex);
    pthread_cond_signal(&gpu_cond);
    pthread_mutex_unlock(&gpu_mutex);
}
