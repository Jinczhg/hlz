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
#include <fcntl.h>
#include <assert.h>
#include <errno.h>

#include <sys/neutrino.h>
#include <screen/screen.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include "gpu.h"
#include "dec.h"
#include "trace.h"
#include "rvm.h"

#define GPU_BUFFER_NUM 2
#define CHANNEL_BUFFER_NUM 3

#define WINDOW_VSYNC 1
#define VERT_SHADER_FILE "./rvm.vert" 
#define FRAG_SHADER_FILE "./rvm.frag"

#define MAX_NUM_TEXELS 2
GLuint textures[MAX_NUM_TEXELS];

static pthread_cond_t gpu_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gpu_mutex = PTHREAD_MUTEX_INITIALIZER;
static OutputBuffer *s_channel_buffers[GPU_BUFFER_NUM][CHANNEL_BUFFER_NUM];
static OutputBuffer *s_buffers[GPU_BUFFER_NUM];
static int s_channel_index[GPU_BUFFER_NUM];

static int s_camera_count = 2;
/* a connection to screen windowing system */
screen_context_t screen_ctx;
/* a native handle for our window */
screen_window_t screen_win;
screen_buffer_t screen_buf[2];

EGLDisplay egldisplay; 
EGLConfig  eglconfig; 
EGLSurface eglsurface; 
EGLContext eglcontext;

static GLint locPosition = 0; 
static GLint locColors = 0; 
static GLint locTexcoord = 0;
static GLint locY = 0; 
static GLint locUV = 0;
static EGLint interval = 1;
static GLuint vertShader = 0; 
static GLuint fragShader = 0;
static GLuint hProgram = 0; 

static int s_width; 
static int s_height; 
static int s_exit = 0;
static int s_tw = 0;
static bool b_gpu_visualble = false;
static rvm_config *s_config = NULL;

static float screen_direction_0_positions[2][18] =
{
    {
	-1.0, -1.0, 0.0,
	 0.2, -1.0, 0.0,
	 0.2,  1.0, 0.0,
	-1.0, -1.0, 0.0,
	 0.2,  1.0, 0.0,
	-1.0,  1.0, 0.0,
    },
    {
	 0.2, -1.0, 0.0,
	 1.0, -1.0, 0.0,
	 1.0,  1.0, 0.0,
	 0.2, -1.0, 0.0,
	 1.0,  1.0, 0.0,
	 0.2,  1.0, 0.0,
    }
};

static float screen_direction_1_positions[2][18] =
{
    {
	-0.2, -1.0, 0.0,
	 1.0, -1.0, 0.0,
	 1.0,  1.0, 0.0,
	-0.2, -1.0, 0.0,
	 1.0,  1.0, 0.0,
	-0.2,  1.0, 0.0,
    },
    {
	-1.0, -1.0, 0.0,
	-0.2, -1.0, 0.0,
	-0.2,  1.0, 0.0,
	-1.0, -1.0, 0.0,
	-0.2,  1.0, 0.0,
	-1.0,  1.0, 0.0,
    }
};


#if 0
static float view0_texCoords [] = 
{
    1.0, 0.0,
    0.0, 0.0,
    0.0, 1.0,
    1.0, 0.0,
    0.0, 1.0,
    1.0, 1.0,
};

static float view1_texCoords [] = 
{
    1.0, 0.0,
    0.0, 0.0,
    0.0, 1.0,
    1.0, 0.0,
    0.0, 1.0,
    1.0, 1.0,
};
#else

#define COORD_X 0.909090909 // 1280 / 1408    ==> IMAGE_WIDTH / padded_width
#define COORD_Y 0.882352941 // 720 / 816      ==> IMAGE_HEIGHT / padded_height

#define COORD_X_RIGHT   0.068181818  // 96 / 1408     ==> padded_width - IMAGE_WIDTH - PADX_H264 / padded_width
#define COORD_X_LEFT    0.977272727  // 1 - 32 / 1408 ==> 1 - PADX_H264 / padded_width

#define COORD_Y_BOTTOM  0.088235294  // 72 / 816      ==> padded_height - IMAGE_HEIGHT - PADY_H264 / padded_height
#define COORD_Y_TOP     0.970588235  // 1 - 24 / 816  ==> 1 - PADY_H264 / padded_height

#define TEX_COORDS_DEFAULT {COORD_X_LEFT, COORD_Y_TOP, \
    COORD_X_RIGHT, COORD_Y_TOP, \
    COORD_X_RIGHT, COORD_Y_BOTTOM, \
    COORD_X_LEFT, COORD_Y_TOP, \
    COORD_X_RIGHT, COORD_Y_BOTTOM, \
    COORD_X_LEFT, COORD_Y_BOTTOM}

static float view0_texCoords_default [] = 
{
    COORD_X_LEFT, COORD_Y_TOP,
    COORD_X_RIGHT, COORD_Y_TOP,
    COORD_X_RIGHT, COORD_Y_BOTTOM,
    COORD_X_LEFT, COORD_Y_TOP,
    COORD_X_RIGHT, COORD_Y_BOTTOM,
    COORD_X_LEFT, COORD_Y_BOTTOM,
};

#if 0
static float view1_texCoords_default [] = 
{
    COORD_X_LEFT, COORD_Y_BOTTOM,
    COORD_X_RIGHT, COORD_Y_BOTTOM,
    COORD_X_RIGHT, COORD_Y_TOP,
    COORD_X_LEFT, COORD_Y_BOTTOM,
    COORD_X_RIGHT, COORD_Y_TOP,
    COORD_X_LEFT, COORD_Y_TOP,
};
#else
static float view1_texCoords_default [] = 
{ 
    COORD_X_LEFT, COORD_Y_TOP,
    COORD_X_RIGHT, COORD_Y_TOP,
    COORD_X_RIGHT, COORD_Y_BOTTOM,
    COORD_X_LEFT, COORD_Y_TOP,
    COORD_X_RIGHT, COORD_Y_BOTTOM,
    COORD_X_LEFT, COORD_Y_BOTTOM,
};
#endif

#endif

static float *view0_positions = screen_direction_0_positions[0];
static float *view1_positions = screen_direction_0_positions[1];

static float view0_normal_angle[] = TEX_COORDS_DEFAULT;
static float view0_wide_angle[] = TEX_COORDS_DEFAULT;
static float view1_normal_angle[] = TEX_COORDS_DEFAULT;
static float view1_wide_angle[] = TEX_COORDS_DEFAULT;

#define GPU_VIEW_NUM 2
#define VIEW_ANGLE_POINT_VALUE_NUM 12
static float view_current_angle[GPU_VIEW_NUM][VIEW_ANGLE_POINT_VALUE_NUM] = {TEX_COORDS_DEFAULT, TEX_COORDS_DEFAULT};
static CAR_CONTROL s_car_control_pre = CAR_MOVE_FORWARD;
static int s_car_manual_time = 0;

void copy_view_angle(float *dst, float *src)
{
    memcpy(dst, src, VIEW_ANGLE_POINT_VALUE_NUM * sizeof(float));
}

void fill_view_angle(float *val, float p11, float p12, float p21, float p22,  float p31, float p32, float p41, float p42)
{
    val[0] = p11 * COORD_X + COORD_X_RIGHT;
    val[1] = p12 * COORD_Y + COORD_Y_BOTTOM;
    val[2] = p21 * COORD_X + COORD_X_RIGHT;
    val[3] = p22 * COORD_Y + COORD_Y_BOTTOM;
    val[4] = p31 * COORD_X + COORD_X_RIGHT;
    val[5] = p32 * COORD_Y + COORD_Y_BOTTOM;
    val[6] = p11 * COORD_X + COORD_X_RIGHT;
    val[7] = p12 * COORD_Y + COORD_Y_BOTTOM;
    val[8] = p31 * COORD_X + COORD_X_RIGHT;
    val[9] = p32 * COORD_Y + COORD_Y_BOTTOM;
    val[10] = p41 * COORD_X + COORD_X_RIGHT;
    val[11] = p42 * COORD_Y + COORD_Y_BOTTOM;
}

static int check_vision(float *val, float step)
{
    if (step > 0)
    {
	if (val[1] + step > COORD_Y_TOP
		|| val[3] + step > COORD_Y_TOP
		|| val[5] + step > COORD_Y_TOP
		|| val[7] + step > COORD_Y_TOP
		|| val[9] + step > COORD_Y_TOP
		|| val[11] + step > COORD_Y_TOP)
	{
	    return -1;
	}
    }
    else
    {
        if (val[1] + step < COORD_Y_BOTTOM
		|| val[3] + step < COORD_Y_BOTTOM
		|| val[5] + step < COORD_Y_BOTTOM
		|| val[7] + step < COORD_Y_BOTTOM
		|| val[9] + step < COORD_Y_BOTTOM
		|| val[11] + step < COORD_Y_BOTTOM)
	{
	    return -1;
	}
    }

    return 0;
}

static void view_set_texcoord(CAR_CONTROL control)
{
#if 0
    if (control == CAR_MOVE_FORWARD)
    {
	copy_view_angle(view_current_angle[0], view0_normal_angle); //rear normal angle
	copy_view_angle(view_current_angle[1], view1_wide_angle); //rear wide angle
    }
    else if (control == CAR_TURN_LEFT)
    {
	if (s_config->car_side == CAR_SIDE_LEFT)
	{
            copy_view_angle(view_current_angle[0], view0_normal_angle); //rear normal angle
	    copy_view_angle(view_current_angle[1], view1_normal_angle); //front normal angle
	}
	else if (s_config->car_side == CAR_SIDE_RIGHT)
	{
            copy_view_angle(view_current_angle[0], view0_wide_angle); //rear wide angle
	    copy_view_angle(view_current_angle[1], view1_wide_angle); //rear wide angle
	}
    }
    else if (control == CAR_TURN_RIGHT)
    {
        if (s_config->car_side == CAR_SIDE_LEFT)
	{
            copy_view_angle(view_current_angle[0], view0_wide_angle); //rear wide angle
	    copy_view_angle(view_current_angle[1], view1_wide_angle); //rear wide angle
	}
	else if (s_config->car_side == CAR_SIDE_RIGHT)
	{
            copy_view_angle(view_current_angle[0], view0_normal_angle); //rear normal angle
	    copy_view_angle(view_current_angle[1], view1_normal_angle); //front normal angle
	}
    }
    else if (control == CAR_REVERSING)
    {
        copy_view_angle(view_current_angle[0], view0_wide_angle); //rear wide angle
	copy_view_angle(view_current_angle[1], view1_wide_angle); //rear wide angle
    }
#else
    if (control == CAR_MOVE_FORWARD)
    {
        copy_view_angle(view_current_angle[0], view0_normal_angle); //rear normal angle
	copy_view_angle(view_current_angle[1], view1_wide_angle); //rear wide angle
    }
    else if ( (control == CAR_TURN_LEFT && s_config->car_side == CAR_SIDE_RIGHT)
	    || (control == CAR_TURN_RIGHT && s_config->car_side == CAR_SIDE_LEFT)
	    || control == CAR_REVERSING)
    {
        copy_view_angle(view_current_angle[0], view0_wide_angle); //rear wide angle
	copy_view_angle(view_current_angle[1], view1_wide_angle); //rear wide angle
    }
    else if ( (control == CAR_TURN_LEFT && s_config->car_side == CAR_SIDE_LEFT)
	    || (control == CAR_TURN_RIGHT && s_config->car_side == CAR_SIDE_RIGHT) )
    {
	copy_view_angle(view_current_angle[0], view0_normal_angle); //rear normal angle
	copy_view_angle(view_current_angle[1], view1_normal_angle); //front normal angle
    }
#endif
}

static void view_manual_set_texcoord(CAR_MANUAL_VISION manual)
{
    float step = 0.0f;

    //DEBUG("s_car_manual_time=%d\n", s_car_manual_time);

    if (manual == CAR_MANUAL_VISION_LEFT)
    {
	if (s_car_manual_time <= -5)
	{
	    return;
	}

	s_car_manual_time -= 1;

	step = -s_config->vision_move_step;
    }
    else if (manual == CAR_MANUAL_VISION_RIGHT)
    {
	if (s_car_manual_time >= 5)
	{
	    return;
	}

	s_car_manual_time += 1;

	step = s_config->vision_move_step;
    }

    step = step / IMAGE_HEIGHT * COORD_Y;

    if (check_vision(view_current_angle[0], step) == 0)
    {
        view_current_angle[0][1] += step;
	view_current_angle[0][3] += step;
	view_current_angle[0][5] += step;
	view_current_angle[0][7] += step;
	view_current_angle[0][9] += step;
	view_current_angle[0][11] += step;
    }

    if (check_vision(view_current_angle[1], step) == 0)
    {
        view_current_angle[1][1] += step;
	view_current_angle[1][3] += step;
	view_current_angle[1][5] += step;
	view_current_angle[1][7] += step;
	view_current_angle[1][9] += step;
	view_current_angle[1][11] += step;
    }
}

void save_view_angle_file()
{
    char buf[1024] = {0};
    int fd = -1;
    memset(buf, 0, 1024);

    sprintf(buf, 
	    "view_0_normal:%f,%f;%f,%f;%f,%f;%f,%f\n"
	    "view_0_wide:%f,%f;%f,%f;%f,%f;%f,%f\n"
	    "view_1_normal:%f,%f;%f,%f;%f,%f;%f,%f\n"
	    "view_1_wide:%f,%f;%f,%f;%f,%f;%f,%f\n",
	    
	    (view0_normal_angle[0] - COORD_X_RIGHT) / COORD_X, (view0_normal_angle[1] - COORD_Y_BOTTOM) / COORD_Y, 
	    (view0_normal_angle[2] - COORD_X_RIGHT) / COORD_X, (view0_normal_angle[3] - COORD_Y_BOTTOM) / COORD_Y, 
	    (view0_normal_angle[4] - COORD_X_RIGHT) / COORD_X, (view0_normal_angle[5] - COORD_Y_BOTTOM) / COORD_Y, 
	    (view0_normal_angle[10] - COORD_X_RIGHT) / COORD_X, (view0_normal_angle[11] - COORD_Y_BOTTOM) / COORD_Y,

            (view0_wide_angle[0] - COORD_X_RIGHT) / COORD_X, (view0_wide_angle[1] - COORD_Y_BOTTOM) / COORD_Y, 
	    (view0_wide_angle[2] - COORD_X_RIGHT) / COORD_X, (view0_wide_angle[3] - COORD_Y_BOTTOM) / COORD_Y, 
	    (view0_wide_angle[4] - COORD_X_RIGHT) / COORD_X, (view0_wide_angle[5] - COORD_Y_BOTTOM) / COORD_Y, 
	    (view0_wide_angle[10] - COORD_X_RIGHT) / COORD_X, (view0_wide_angle[11] - COORD_Y_BOTTOM) / COORD_Y,

            (view1_normal_angle[0] - COORD_X_RIGHT) / COORD_X, (view1_normal_angle[1] - COORD_Y_BOTTOM) / COORD_Y, 
	    (view1_normal_angle[2] - COORD_X_RIGHT) / COORD_X, (view1_normal_angle[3] - COORD_Y_BOTTOM) / COORD_Y, 
	    (view1_normal_angle[4] - COORD_X_RIGHT) / COORD_X, (view1_normal_angle[5] - COORD_Y_BOTTOM) / COORD_Y, 
	    (view1_normal_angle[10] - COORD_X_RIGHT) / COORD_X, (view1_normal_angle[11] - COORD_Y_BOTTOM) / COORD_Y,

            (view1_wide_angle[0] - COORD_X_RIGHT) / COORD_X, (view1_wide_angle[1] - COORD_Y_BOTTOM) / COORD_Y, 
	    (view1_wide_angle[2] - COORD_X_RIGHT) / COORD_X, (view1_wide_angle[3] - COORD_Y_BOTTOM) / COORD_Y, 
	    (view1_wide_angle[4] - COORD_X_RIGHT) / COORD_X, (view1_wide_angle[5] - COORD_Y_BOTTOM) / COORD_Y, 
	    (view1_wide_angle[10] - COORD_X_RIGHT) / COORD_X, (view1_wide_angle[11] - COORD_Y_BOTTOM) / COORD_Y);


    fd = open("/tmp/view_angle.txt", O_WRONLY | O_CREAT);
    if (fd > 0)
    {
        write(fd, buf, strlen(buf));
    }
}

void parse_view_angle_file()
{
    char file[256] = {0};
    char buf[1024] = {0};
    char *line = NULL;
    FILE *fp = NULL;
    float p11, p12, p21, p22,  p31, p32, p41, p42;

    if (access("/tmp/view_angle.txt", F_OK) == 0)
    {
	sprintf(file, "/tmp/view_angle.txt");
    }
    else
    {
        sprintf(file, "./view_angle_%s.txt", s_config->car_side == CAR_SIDE_LEFT ? "left" : "right");
    }

    fp = fopen(file, "r");
    if (!fp)
    {
	ERROR("open %s error: %s", file, strerror(errno));
	return;
    }
    
    while (fgets(buf, 1023, fp) != NULL)
    {
	if ((line = strstr(buf, "view_0_normal:")))
	{
	    sscanf(line, "view_0_normal:%f,%f;%f,%f;%f,%f;%f,%f", &p11, &p12, &p21, &p22, &p31, &p32, &p41, &p42);
	    fill_view_angle(view0_normal_angle, p11, p12, p21, p22,  p31, p32, p41, p42);
	}
	else if ((line = strstr(buf, "view_0_wide:")))
	{
	    sscanf(line, "view_0_wide:%f,%f;%f,%f;%f,%f;%f,%f", &p11, &p12, &p21, &p22, &p31, &p32, &p41, &p42);
	    fill_view_angle(view0_wide_angle, p11, p12, p21, p22,  p31, p32, p41, p42);
	}
	else if ((line = strstr(buf, "view_1_normal:")))
	{
	    sscanf(line, "view_1_normal:%f,%f;%f,%f;%f,%f;%f,%f", &p11, &p12, &p21, &p22, &p31, &p32, &p41, &p42);
	    fill_view_angle(view1_normal_angle, p11, p12, p21, p22,  p31, p32, p41, p42);
	}
	else if ((line = strstr(buf, "view_1_wide:")))
	{
	    sscanf(line, "view_1_wide:%f,%f;%f,%f;%f,%f;%f,%f", &p11, &p12, &p21, &p22, &p31, &p32, &p41, &p42);
	    fill_view_angle(view1_wide_angle, p11, p12, p21, p22,  p31, p32, p41, p42);
	}
    }

    fclose(fp);
}

int gpu_init(rvm_config *config)
{
    int i = 0, j = 0;
    const char *tok;
    int tw = decode_padded_width() * decode_padded_height() * 3 / 2;
    s_tw = tw;
    s_config = config;

    DEBUG("init......");
    
    parse_view_angle_file();

    if (config->camera_count > 0)
    {
        s_camera_count = config->camera_count;
    }

    if (config->screen_direction == 0)
    {
        view0_positions = screen_direction_0_positions[0];
	view1_positions = screen_direction_0_positions[1];
    }
    else
    {
	view0_positions = screen_direction_1_positions[0];
	view1_positions = screen_direction_1_positions[1];
    }

    for (i = 0; i < GPU_BUFFER_NUM; i++)
    {
        for (j = 0; j < CHANNEL_BUFFER_NUM; j++)
        {
	    s_channel_buffers[i][j] = output_alloc(1, decode_padded_width(), decode_padded_height());

	    if (!s_channel_buffers[i][j])
	    {
	        i--;
	        while (i != -1)
	        {
	            j--;
	            while (j != -1)
	            {
		       output_free(s_channel_buffers[i][j], 1);
		       s_channel_buffers[i][j] = NULL;
		       j--;
		    }
		    i--;
	        }
	        return -1;
	    }
	    memset(s_channel_buffers[i][j]->buf, 128, tw);
	}
	s_buffers[i] = s_channel_buffers[i][0];
	s_channel_index[i] = 0;
    }

    DEBUG("init OK");
    return 0;
}

void gpu_deinit()
{
    int i = 0, j = 0;

    for (i = 0; i < GPU_BUFFER_NUM; i++)
    {
        for (j = 0; j < CHANNEL_BUFFER_NUM; j++)
	if (!s_channel_buffers[i][j])
	{
	    free(s_channel_buffers[i][j]);
	    s_channel_buffers[i][j] = NULL;
	}
    }

    DEBUG("deinit OK");
}

OutputBuffer* gpu_get_buffer(int channel)
{
    int index = s_channel_index[channel];
    s_channel_index[channel] = ++s_channel_index[channel] % CHANNEL_BUFFER_NUM;
    return s_channel_buffers[channel][index];
}

static int compileShader(const char *fileName, GLuint shaderNum)
{
    FILE *fp = NULL;
    char *shaderSource = NULL;
    int length = 0;
    GLint compiled = 0;
    fp = fopen(fileName, "rb");

    if (fp == NULL)
    {
        fprintf(stderr, "fopen error:%s\n", strerror(errno));
	return -1;
    }

    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    shaderSource = (char*)malloc(length);
    if (shaderSource == NULL)
    {
        fprintf(stderr, "malloc error:%s\n", strerror(errno));
	return -1;
    }

    fread(shaderSource, length, 1, fp);

    glShaderSource(shaderNum, 1, (const char**)&shaderSource, &length);
    glCompileShader(shaderNum);

    free(shaderSource);
    fclose(fp);
    glGetShaderiv(shaderNum, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
	GLint errorBufSize, errorLength;
	glGetShaderiv(shaderNum, GL_INFO_LOG_LENGTH, &errorBufSize);
	char *infoLog = (char*)malloc(errorBufSize + 1);
	if (infoLog)
	{
	    glGetShaderInfoLog(shaderNum, errorBufSize, &errorLength, infoLog);
	    infoLog[errorBufSize] = '\0';
	    fprintf(stderr, "%s\n", infoLog);
	    
	    free(infoLog);
	}

	return -1;
    }

    return 0;
}

static GLuint buildShaders(const char *vs_file, const char *fs_file)
{
    GLuint program = -1;
    GLint linked = 0;

    vertShader = glCreateShader(GL_VERTEX_SHADER);
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    if (compileShader(vs_file, vertShader) == -1)
    {
	fprintf(stderr, "vert shader compile failed.\n");
	goto Error;
    }

    if (compileShader(fs_file, fragShader) == -1)
    {
	fprintf(stderr, "frag shader compile failed.\n");
	goto Error;
    }

    program = glCreateProgram();

    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
	GLint errorBufSize, errorLength;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &errorBufSize);
	char *infoLog = (char*)malloc(errorBufSize + 1);
	if (infoLog)
	{
	    glGetProgramInfoLog(program, errorBufSize, &errorLength, infoLog);
	    infoLog[errorBufSize] = '\0';
	    fprintf(stderr, "%s\n", infoLog);
	    
	    free(infoLog);
	}

	program = -1;
	goto Error;
    }

    glUseProgram(program);

Error:
    return program;
}

static GLuint bindTextureY(GLuint texture, const char *buffer, GLuint w, GLuint h)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

static GLuint bindTextureUV(GLuint texture, const char *buffer, GLuint w, GLuint h)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, w, h, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

static int init()
{
    const EGLint s_configAttribs[] =
    {
	EGL_RED_SIZE,   8,
        EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE,  8,
	EGL_ALPHA_SIZE, EGL_DONT_CARE,
	EGL_DEPTH_SIZE, EGL_DONT_CARE,
	EGL_STENCIL_SIZE, EGL_DONT_CARE,
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	EGL_SAMPLE_BUFFERS, 0,
	EGL_NONE
    };
    
    const EGLint eglSurfaceAttrs[] =
    {
        EGL_RENDER_BUFFER,  EGL_BACK_BUFFER,
        EGL_NONE
    };
    
    const EGLint eglContextAttribList[] = 
    {
	EGL_CONTEXT_CLIENT_VERSION, 2,
	EGL_NONE
    };

    EGLint numconfigs;

    int ret = 0;
    int size[2] = {WIN_WIDTH, WIN_HEIGHT};
    int format = SCREEN_FORMAT_RGBA8888;
    int usage = SCREEN_USAGE_OPENGL_ES2;

    screen_create_context(&screen_ctx, 0);
    screen_create_window(&screen_win, screen_ctx);
    screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SOURCE_SIZE, size);
    screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SIZE, size);
    screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_FORMAT, &format);
    screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_USAGE, &usage);
    screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SWAP_INTERVAL, &interval);
    screen_create_window_buffers(screen_win, 1);
    screen_get_window_property_pv(screen_win, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)screen_buf);

    egldisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(egldisplay, NULL, NULL);
    assert(eglGetError() == EGL_SUCCESS);
    
    eglChooseConfig(egldisplay, s_configAttribs, &eglconfig, 1, &numconfigs);
    assert(eglGetError() == EGL_SUCCESS);
    assert(numconfigs == 1);

    eglsurface = eglCreateWindowSurface(egldisplay, eglconfig, screen_win, eglSurfaceAttrs);

    eglcontext = eglCreateContext(egldisplay, eglconfig, EGL_NO_CONTEXT, eglContextAttribList);
    
    ret = eglMakeCurrent(egldisplay, eglsurface, eglsurface, eglcontext);
    if (ret == EGL_FALSE)
    {
	ERROR("eglMakeCurrent failed: 0x%08X\n", ret);
	return -1;
    }
    
    if (WINDOW_VSYNC)
    {
        eglSwapInterval(egldisplay, 1);
    }

    hProgram = buildShaders(VERT_SHADER_FILE, FRAG_SHADER_FILE);
    
    if (hProgram == -1)
    {
        ERROR("buildShaders failed\n");
        return -1;
    }

    eglQuerySurface(egldisplay, eglsurface, EGL_WIDTH, &s_width);
    eglQuerySurface(egldisplay, eglsurface, EGL_HEIGHT, &s_height);
    
    locPosition = glGetAttribLocation(hProgram, "a_position");
    locTexcoord = glGetAttribLocation(hProgram, "a_texCoord");
    
    locY = glGetUniformLocation(hProgram, "y_texture");
    locUV = glGetUniformLocation(hProgram, "uv_texture");

    glEnableVertexAttribArray(locPosition);
    glEnableVertexAttribArray(locTexcoord);

    glGenTextures(MAX_NUM_TEXELS, textures);
    glEnable(GL_TEXTURE_2D);

    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);

    return 0;
}

static void render(char *texture_buf0, char *texture_buf1, int width, int height)
{
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    //view0
    glActiveTexture(GL_TEXTURE0);
    bindTextureY(textures[0], texture_buf0, width, height);
    
    glActiveTexture(GL_TEXTURE1);
    bindTextureUV(textures[1], texture_buf0 + width * height, width / 2, height / 2);
    
    glVertexAttribPointer(locPosition, 3, GL_FLOAT, GL_FALSE, 0, view0_positions);
    glVertexAttribPointer(locTexcoord, 2, GL_FLOAT, GL_FALSE, 0, view_current_angle[0]);
    
    glUniform1i(locY, 0);
    glUniform1i(locUV, 1);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //view1
    glActiveTexture(GL_TEXTURE0);
    bindTextureY(textures[0], texture_buf1, width, height);
    
    glActiveTexture(GL_TEXTURE1);
    bindTextureUV(textures[1], texture_buf1 + width * height, width / 2, height / 2);
    
    glVertexAttribPointer(locPosition, 3, GL_FLOAT, GL_FALSE, 0, view1_positions);
    glVertexAttribPointer(locTexcoord, 2, GL_FLOAT, GL_FALSE, 0, view_current_angle[1]);
    
    glUniform1i(locY, 0);
    glUniform1i(locUV, 1);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //flush front and rear
    glFlush();
}

void* gpu_thread(void *arg)
{
    uint32_t padded_width = decode_padded_width(); 
    uint32_t padded_height = decode_padded_height();
    int i = 0;
    
    DEBUG("padded_width = %d, padded_height= %d", padded_width, padded_height);
    
    init();

    s_car_control_pre = s_config->car_control;
    view_set_texcoord(s_config->car_control);
    s_car_manual_time = 0;

    DEBUG("gpu thread is ready");

    while (!s_exit)
    {
	pthread_mutex_lock(&gpu_mutex);
	pthread_cond_wait(&gpu_cond, &gpu_mutex);     
	pthread_mutex_unlock(&gpu_mutex);

	if (s_exit)
	{
	    break;
	}

	if (s_config->car_control != s_car_control_pre)
	{
	    s_car_control_pre = s_config->car_control;
	    view_set_texcoord(s_config->car_control);
	    s_car_manual_time = 0;
	}

	if (s_config->car_vision != CAR_MANUAL_VISION_NUM)
	{
	    view_manual_set_texcoord(s_config->car_vision);
	    s_config->car_vision = CAR_MANUAL_VISION_NUM;
	}
	
	if (s_config->car_control == CAR_TURN_LEFT && s_config->car_side == CAR_SIDE_LEFT)
	{
	    render(s_buffers[s_config->channel_rear]->buf, s_buffers[s_config->channel_front]->buf, padded_width, padded_height);
	}
	else if (s_config->car_control == CAR_TURN_RIGHT && s_config->car_side == CAR_SIDE_RIGHT)
	{
	    render(s_buffers[s_config->channel_rear]->buf, s_buffers[s_config->channel_front]->buf, padded_width, padded_height);
	}
	else
	{
	    render(s_buffers[s_config->channel_rear]->buf, s_buffers[s_config->channel_rear]->buf, padded_width, padded_height);
	}

        eglSwapBuffers(egldisplay, eglsurface);
    }


    DEBUG("gpu thread exit");
    return 0;
}

void gpu_signal(int channel, OutputBuffer *buffer)
{
    static int frame_cnt = 0;
    s_buffers[channel] = buffer;
    
    pthread_mutex_lock(&gpu_mutex);

    if (++frame_cnt == s_camera_count)
    {
	pthread_cond_signal(&gpu_cond);
	frame_cnt = 0;
    }

    pthread_mutex_unlock(&gpu_mutex);
}

void gpu_exit()
{
    s_exit = 1;
    pthread_mutex_lock(&gpu_mutex);
    pthread_cond_signal(&gpu_cond);
    pthread_mutex_unlock(&gpu_mutex);
}
