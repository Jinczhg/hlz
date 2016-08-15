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
#include <errno.h>
#include <assert.h>

#include <sys/neutrino.h>
#include <screen/screen.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include "gpu.h"
#include "dec.h"
#include "trace.h"
#include "sv.h"
#include "stitchalgo.h"

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

#ifdef CALC_ALGO_TIME
static int s_cnt_c = 0;
static struct timeval tend;
static struct timeval tstart;

static struct timeval start_s;
static struct timeval end_s;
static unsigned int s_time_min = 0xFFFFFFFF, s_time_max = 0, s_time = 0;
#endif

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

static int m_width; 
static int m_height;
static unsigned char *g_buffer;

#define WINDOW_VSYNC 1
#define VERT_SHADER_FILE "./gpu.vert" 
#define FRAG_SHADER_FILE "./gpu.frag"

static GLint locPosition = 0; 
static GLint locColors = 0; 
static GLint locTexcoord = 0; 
static GLint locSamplerY = 0; 
static GLint locSamplerUV = 0;
static GLuint vertShader = 0; 
static GLuint fragShader = 0; 
static GLuint hProgram = 0; 
static GLuint gTexObj = 0; 

unsigned int d8NumPositions = 6;

float d8Positions[] =
{
    -1.0, -1.0, 0.0,
     1.0, -1.0, 0.0,
     1.0,  1.0, 0.0,
    -1.0, -1.0, 0.0,
     1.0,  1.0, 0.0,
    -1.0,  1.0, 0.0,
};

float d8TexCoords[] =
{
    1.0, 0.0,
    0.0, 0.0,
    0.0, 1.0,
    1.0, 0.0,
    0.0, 1.0,
    1.0, 1.0,
};

#define MAX_NUM_TEXELS 2
GLuint texture[MAX_NUM_TEXELS]; 

/* a connection to screen windowing system */
screen_context_t screen_ctx;
/* a native handle for our window */
screen_window_t screen_win;
screen_buffer_t screen_buf[2];

EGLDisplay egldisplay; 
EGLConfig  eglconfig; 
EGLSurface eglsurface; 
EGLContext eglcontext; 
EGLint interval = 1;

static unsigned char* readYUV(const char *path)
{
    FILE *fp = NULL;
    char *buffer = NULL;
    int length = 0;
    fp = fopen(path, "rb");

    if (fp == NULL)
    {
	goto Out;
    }

    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer = (char*)malloc(length);
    if (buffer == NULL)
    {
	goto Out;
    }

    fread(buffer, length, 1, fp);
    fclose(fp);

Out:
    return buffer;
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
    int size[2] = {SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT};
    int format = SCREEN_FORMAT_RGBA8888;//SCREEN_FORMAT_NV12;
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
    
    //eglBindAPI(EGL_OPENGL_ES_API);
    
    eglChooseConfig(egldisplay, s_configAttribs, &eglconfig, 1, &numconfigs);
    assert(eglGetError() == EGL_SUCCESS);
    assert(numconfigs == 1);

    eglsurface = eglCreateWindowSurface(egldisplay, eglconfig, screen_win, eglSurfaceAttrs);

    eglcontext = eglCreateContext(egldisplay, eglconfig, EGL_NO_CONTEXT, eglContextAttribList);
    
    ret = eglMakeCurrent(egldisplay, eglsurface, eglsurface, eglcontext);
    if (ret == EGL_FALSE)
    {
	fprintf(stderr, "eglMakeCurrent failed: 0x%08X\n", ret);
	return -1;
    }
    
    if (WINDOW_VSYNC)
    {
        eglSwapInterval(egldisplay, 1);
    }

    hProgram = buildShaders(VERT_SHADER_FILE, FRAG_SHADER_FILE);
    
    if (hProgram == -1)
    {
        fprintf(stderr, "buildShaders failed\n");
        return -1;
    }

    locPosition = glGetAttribLocation(hProgram, "a_position");
    locTexcoord = glGetAttribLocation(hProgram, "a_texCoord");
    
    locSamplerY = glGetUniformLocation(hProgram, "y_texture");
    locSamplerUV = glGetUniformLocation(hProgram, "uv_texture");

    glEnableVertexAttribArray(locPosition);
    glEnableVertexAttribArray(locTexcoord);

    glVertexAttribPointer(locPosition, 3, GL_FLOAT, GL_FALSE, 0, d8Positions);
    glVertexAttribPointer(locTexcoord, 2, GL_FLOAT, GL_FALSE, 0, d8TexCoords);

    eglQuerySurface(egldisplay, eglsurface, EGL_WIDTH, &m_width);
    eglQuerySurface(egldisplay, eglsurface, EGL_HEIGHT, &m_height);

    glGenTextures(MAX_NUM_TEXELS, texture);
    glEnable(GL_TEXTURE_2D);

    g_buffer = malloc(SV_IMAGE_WIDTH * SV_IMAGE_HEIGHT * 3 / 2);

    glViewport(0, 0, SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT);

    glActiveTexture(GL_TEXTURE0);
    bindTextureY(texture[0], g_buffer, SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT);
    glActiveTexture(GL_TEXTURE1);
    bindTextureUV(texture[1], g_buffer + SV_IMAGE_WIDTH * SV_IMAGE_HEIGHT, SV_IMAGE_WIDTH / 2, SV_IMAGE_HEIGHT / 2);

    return 0;
}

static void deinit()
{
}

static void render()
{
    glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, GL_TEXTURE0);
    bindTextureY(texture[0], g_buffer, SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT);
    glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, GL_TEXTURE1);
    bindTextureUV(texture[1], g_buffer + SV_IMAGE_WIDTH * SV_IMAGE_HEIGHT, SV_IMAGE_WIDTH / 2, SV_IMAGE_HEIGHT / 2);
    glUniform1i(locSamplerY, 0);
    glUniform1i(locSamplerUV, 1);
    glDrawArrays(GL_TRIANGLES, 0, d8NumPositions);
    glFlush();
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
    int data = 0x01;
    //ThreadCtl(_NTO_TCTL_RUNMASK, &data);

    init();
    s_output.bufYUV = (uint32_t)g_buffer;
    s_screen_win_p = &screen_win;
    s_screen_buf_p = screen_buf;

    uint32_t in_stride = decode_padded_width(); 
    uint32_t y_offset = decode_y_offset();
    uint32_t uv_offset = decode_uv_offset();
    float diff = 0;

    initStitching(in_stride, y_offset, uv_offset, SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT);
    
#ifdef CALC_ALGO_TIME    
    gettimeofday(&tstart, NULL);
#endif

    while (!s_exit)
    {
	pthread_mutex_lock(&gpu_mutex);
	pthread_cond_wait(&gpu_cond, &gpu_mutex);     
	pthread_mutex_unlock(&gpu_mutex);

	if (s_exit)
	{
	    break;
	}

#ifdef CALC_ALGO_TIME
        gettimeofday(&start_s, NULL);
#endif

	//doStitching here
	doStitching(&s_front, &s_rear, &s_left, &s_right, &s_output);

#ifdef CALC_ALGO_TIME
	gettimeofday(&end_s, NULL);
        s_time = (end_s.tv_sec - start_s.tv_sec) * 1000000 + (end_s.tv_usec - start_s.tv_usec);
	if (s_time > s_time_max)
	{
	    s_time_max = s_time;
	    DEBUG("doStitching max time us:%d", s_time_max);
	}
	else if (s_time < s_time_min)
	{
	    s_time_min = s_time;
	    DEBUG("doStitching min time us:%d", s_time_min);
	}
#endif

	render();
        eglSwapBuffers(egldisplay, eglsurface);

#ifdef CALC_ALGO_TIME
	if (s_cnt_c++ == 150)
	{
            gettimeofday(&tend, NULL);
	    diff = (tend.tv_sec - tstart.tv_sec) + (tend.tv_usec - tstart.tv_usec) / 1000000;
	    DEBUG("%f fps, recv average time us:%d, decode average time us:%d", 150.0 / diff, get_average_recv_time(), get_average_decode_time());
	    s_cnt_c = 0;
	    tstart = tend;
	}
#endif
    }

    DEBUG("gpu thread exit");
    return 0;
}

void gpu_signal()
{
    static int frame_cnt = 0;

    pthread_mutex_lock(&gpu_mutex);
    if (++frame_cnt == CHANNEL_NUM_MAX)
    {
	pthread_cond_signal(&gpu_cond);
	frame_cnt = 0;
    }
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
