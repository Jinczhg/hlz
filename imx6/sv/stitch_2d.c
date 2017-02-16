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
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>
#include <assert.h> 
#include <sys/time.h>
#include <sys/resource.h>

#include <GLES2/gl2.h> 
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglvivante.h>

#include <g2d.h>

#include "stitch_2d.h"
#include "dec.h"
#include "trace.h"
#include "sv.h"
#include "stitchalgo.h"

#define VERT_SHADER_FILE "./mxc_vgpu.vert" 
#define FRAG_SHADER_FILE "./mxc_vgpu.frag"


static EGLNativeDisplayType native_display; 
static EGLNativeWindowType  native_window; 
static EGLDisplay egldisplay; 
static EGLConfig eglconfig; 
static EGLSurface eglsurface; 
static EGLContext eglcontext; 

// Member Variables, attribute and uniform 
static GLint locVertices = 0; 
static GLint locTransformMat = 0; 
static GLint locProjectionMat = 0; 
static GLint locTexcoord = 0; 
static GLint locSampler = 0; 
static GLuint vertShader = 0; 
static GLuint fragShader = 0; 
static GLuint hProgram = 0; 

static int m_width; 
static int m_height; 
 
struct NativeDisplay
{
    int width;
    int height; 
};

static struct NativeDisplay nativeDisplay = {
    .width = 0,
    .height = 0, 
};

typedef struct
{
    unsigned int yVirt;
    unsigned int cbVirt;
    unsigned int crVirt;
    unsigned int yPhys;
    unsigned int cbPhys;
    unsigned int crPhys;
    int width;
    int height;
} gpu_frame_buf;

#define MAX_NUM_TEXELS 1
static GLuint texture[MAX_NUM_TEXELS];
static GLvoid *pTexel[MAX_NUM_TEXELS][3];
static gpu_frame_buf s_gpuFrameBuffer[MAX_NUM_TEXELS];
static GLint s_egl_texture[2] = {GL_TEXTURE0, GL_TEXTURE1};

static unsigned int d8NumVerts = 6;

static float d8Verts [] = {
  -1.0, -1.0, 0.0,
   1.0, -1.0, 0.0,
   1.0,  1.0, 0.0,
  -1.0, -1.0, 0.0,
   1.0,  1.0, 0.0,
  -1.0,  1.0, 0.0,
};

/* to resolve the mirror problem */
static float d8TexCoords [] = {
  1.0, 0.0,
  0.0, 0.0,
  0.0, 1.0,
  1.0, 0.0,
  0.0, 1.0,
  1.0, 1.0,
};

// Start with an identity matrix. 
static GLfloat projectionMatrix[16] = 
{ 
    1.0f, 0.0f, 0.0f, 0.0f, 
    0.0f, 1.0f, 0.0f, 0.0f, 
    0.0f, 0.0f, 1.0f, 0.0f, 
    0.0f, 0.0f, 0.0f, 1.0f 
}; 

static GLfloat transformMatrix[16] = 
{ 
    1.0f, 0.0f, 0.0f, 0.0f, 
    0.0f, 1.0f, 0.0f, 0.0f, 
    0.0f, 0.0f, 1.0f, 0.0f, 
    0.0f, 0.0f, 0.0f, 1.0f 
};  

static pthread_cond_t stitch_2d_conf = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t stitch_2d_mutex = PTHREAD_MUTEX_INITIALIZER;
static unsigned long s_src[CHANNEL_NUM_MAX] = {0};
static int s_out_width = 0;
static int s_out_height = 0;
static int s_ready = 0;

extern sv_config g_config;
extern int g_exit;

static int compileShader(const char * fileName, GLuint shaderNum) 
{ 
    FILE *fp = NULL; 
    char *shaderSource = NULL; 
    int length = 0; 
    GLint compiled = 0; 
    int ret = 0;
     
    fp = fopen(fileName, "rb"); 
 
    if (fp == NULL) 
    {
	ERROR("fopen error: %s", strerror(errno));
        ret = -1;
	goto out;
    } 
 
    fseek(fp, 0, SEEK_END); 
    length = ftell(fp); 
    fseek(fp, 0 ,SEEK_SET); 
 
    shaderSource = (char*)malloc(sizeof(char) * length); 
    if (shaderSource == NULL) 
    { 
        ERROR("malloc error"); 
        ret = -1;
	goto out;
    } 
 
    fread(shaderSource, length, 1, fp); 
 
    glShaderSource(shaderNum, 1, (const char**)&shaderSource, &length); 
    glCompileShader(shaderNum); 
 
    glGetShaderiv(shaderNum, GL_COMPILE_STATUS, &compiled); 
    if (!compiled) 
    { 
        // Retrieve error buffer size. 
        GLint errorBufSize, errorLength; 
        glGetShaderiv(shaderNum, GL_INFO_LOG_LENGTH, &errorBufSize); 
 
        char *infoLog = (char*)malloc(errorBufSize * sizeof(char) + 1); 
        if (infoLog) 
        { 
            // Retrieve error. 
            glGetShaderInfoLog(shaderNum, errorBufSize, &errorLength, infoLog); 
            infoLog[errorBufSize] = '\0'; 
            DEBUG("%s", infoLog); 
 
            free(infoLog); 
        }
        
        ret = -1;
	goto out;
    } 
 
out:
    if (fp)
    {
	fclose(fp);
    }

    if (shaderSource)
    {
	free(shaderSource);
    }

    return ret; 
} 
 
static GLuint buildShaders(const char * vs_file, const char * fs_file) 
{ 
    GLuint program = -1; 
    GLint linked = false; 

    vertShader = glCreateShader(GL_VERTEX_SHADER); 
    fragShader = glCreateShader(GL_FRAGMENT_SHADER); 
 
    if (compileShader(vs_file, vertShader) != 0) 
    {
	DEBUG("vert shader compile failed."); 
	goto Error; 
    } 
 
    if (compileShader(fs_file, fragShader) != 0) 
    {
	DEBUG("frag shader compile failed.");
	goto Error; 
    } 
 
    program = glCreateProgram(); 
 
    glAttachShader(program, vertShader); 
    glAttachShader(program, fragShader); 
 
    glLinkProgram(program); 
 
    // Check if linking succeeded. 
    glGetProgramiv(program, GL_LINK_STATUS, &linked); 
    if (!linked) 
    { 
        DEBUG("Link failed."); 
        // Retrieve error buffer size. 
        GLint errorBufSize, errorLength; 
        glGetShaderiv(program, GL_INFO_LOG_LENGTH, &errorBufSize); 
 
        char * infoLog = (char*)malloc(errorBufSize * sizeof(char) + 1); 
        if (!infoLog) 
        { 
            // Retrieve error. 
            glGetProgramInfoLog(program, errorBufSize, &errorLength, infoLog); 
            infoLog[errorBufSize + 1] = '\0'; 
            DEBUG("%s", infoLog); 
 
            free(infoLog); 
        } 
	
	program = -1; 
        goto Error; 
    } 
	 
    glUseProgram(program); 

Error: 
    return program; 
} 

static int alloc_buffer() 
{
    int i = 0;

    glGenTextures(MAX_NUM_TEXELS, texture);

    for (i = 0; i < MAX_NUM_TEXELS; i++)
    {
	glActiveTexture(s_egl_texture[i]);
	glBindTexture(GL_TEXTURE_2D, texture[i]);
	
	glTexDirectVIV(GL_TEXTURE_2D, s_out_width, s_out_height, GL_VIV_NV12, (GLvoid **)&pTexel[i]);
	s_gpuFrameBuffer[i].yVirt = (unsigned int)pTexel[i][0]; 
	s_gpuFrameBuffer[i].cbVirt = (unsigned int)pTexel[i][1]; 
	s_gpuFrameBuffer[i].crVirt = 0;

        s_gpuFrameBuffer[i].width = s_out_width;  
	s_gpuFrameBuffer[i].height = s_out_height;
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	memset(s_gpuFrameBuffer[i].yVirt, 0, s_out_width * s_out_height * 3 / 2);
    }

    return 0;
}

static int init(void) 
{
    int ret = 0;

    static const EGLint s_configAttribs[] = 
    { 
	EGL_RED_SIZE,		5, 
	EGL_GREEN_SIZE, 	6, 
	EGL_BLUE_SIZE,		5, 
	EGL_ALPHA_SIZE, 	0, 
      	EGL_SAMPLES, 		0, 
      	EGL_NONE 
    }; 
 
    EGLint numconfigs;
     
    //get egl display 
    egldisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY); 
    
    //Initialize egl 
    eglInitialize(egldisplay, NULL, NULL); 
    assert(eglGetError() == EGL_SUCCESS); 
    eglBindAPI(EGL_OPENGL_ES_API); 
    eglChooseConfig(egldisplay, s_configAttribs, &eglconfig, 1, &numconfigs); 
    assert(eglGetError() == EGL_SUCCESS); 
    assert(numconfigs == 1); 
    
    native_display = fbGetDisplay(NULL); 
    fbGetDisplayGeometry(native_display, &nativeDisplay.width, &nativeDisplay.height); 

    DEBUG("naiveDisplay width: %d, height: %d", nativeDisplay.width, nativeDisplay.height);

    //native_window  = fbCreateWindow(native_display, 0, 0,
//	    nativeDisplay.width / 2, nativeDisplay.height); 
    native_window  = fbCreateWindow(native_display, SV_STITCH_X, SV_STITCH_Y,
	    SV_STITCH_WIDTH, SV_STITCH_HEIGHT); 
    DEBUG("eglCreateWindowSurface , the window width and height is %d:%d",
	    SV_STITCH_WIDTH, SV_STITCH_HEIGHT); 
    eglsurface = eglCreateWindowSurface(egldisplay, eglconfig, native_window, NULL); 
    assert(eglGetError() == EGL_SUCCESS); 
 
    //create the egl graphics context 
    static const EGLint s_contextAttribList[] =
    { 
	EGL_CONTEXT_CLIENT_VERSION,
	2,
	EGL_NONE
    };

    eglcontext = eglCreateContext( egldisplay, eglconfig, EGL_NO_CONTEXT, s_contextAttribList ); 
    DEBUG("creatcontext"); 
    assert(eglGetError() == EGL_SUCCESS); 
 
    //make the context current 
    ret = eglMakeCurrent(egldisplay, eglsurface, eglsurface, eglcontext); 
    DEBUG("makecurrent"); 
    assert(eglGetError() == EGL_SUCCESS); 
    if (ret == EGL_FALSE)
    {
	ERROR("eglMakeCurrent failed :0x%08x", ret);
    } 
 
    eglQuerySurface(egldisplay, eglsurface, EGL_WIDTH, &m_width);
    eglQuerySurface(egldisplay, eglsurface, EGL_HEIGHT, &m_height); 
 
    hProgram = buildShaders(VERT_SHADER_FILE, FRAG_SHADER_FILE); 
 
    // Grab location of shader attributes. 
    locVertices = glGetAttribLocation(hProgram, "vsVertex"); 
    locTexcoord = glGetAttribLocation(hProgram, "vsTexcoord"); 
    // Transform Matrix is uniform for all vertices here. 
    locTransformMat = glGetUniformLocation(hProgram, "vsTransformMatrix"); 
    locProjectionMat = glGetUniformLocation(hProgram, "vsProjectionMatrix"); 
    locSampler = glGetUniformLocation(hProgram, "fsSampler"); 
 
    // enable vertex arrays to push the data. 
    glEnableVertexAttribArray(locVertices); 
    glEnableVertexAttribArray(locTexcoord); 
 
    glVertexAttribPointer(locVertices, 3, GL_FLOAT, GL_FALSE, 0, d8Verts); 
    glVertexAttribPointer(locTexcoord, 2, GL_FLOAT, GL_FALSE, 0, d8TexCoords); 
 
    glUniformMatrix4fv(locTransformMat, 1, GL_FALSE, transformMatrix); 
 
    alloc_buffer();

    glEnable(GL_TEXTURE_2D);

    return 0; 
} 

static void render(texture) 
{ 
    // Clear background. 
    glClearColor(0.0f, 0.5f, 0.5f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT); 
    glEnable(GL_CULL_FACE); 
    glCullFace( GL_BACK ); 

    glActiveTexture(s_egl_texture[texture]); 
    glUniformMatrix4fv(locTransformMat, 1, GL_FALSE, transformMatrix); 
    glUniformMatrix4fv(locProjectionMat, 1, GL_FALSE, projectionMatrix); 
    glUniform1i(locSampler, texture); 
    glTexDirectInvalidateVIV(GL_TEXTURE_2D); 
    glDrawArrays(GL_TRIANGLES, 0, d8NumVerts); 
    glFlush(); 
}

int stitch_2d_init()
{
    DEBUG("init...");
    //init();
    DEBUG("init OK");
    return 0;
}

void stitch_2d_deinit()
{
    DEBUG("deinit OK");
}

void* stitch_2d_thread(void *arg)
{
    int frames = 0;
    int texture = 0;
	
    initStitching(SV_IMAGE_WIDTH, 0, SV_IMAGE_WIDTH * SV_IMAGE_HEIGHT, SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT);
    getStitchingSize(&s_out_width, &s_out_height);

    YUVBuffer front = {SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, 0 };
    YUVBuffer rear = {SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, 0 };
    YUVBuffer left = {SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, 0 };
    YUVBuffer right = {SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, 0 };
    YUVBuffer output = {s_out_width, s_out_height, 0 };
    
    int tw = SV_IMAGE_WIDTH * SV_IMAGE_HEIGHT * 3 / 2;
    
    struct g2d_buf *buf;
    
    buf = g2d_alloc(tw, 1);
    if (!buf)
    {
        ERROR("Fail to allocate physical memory for src buffer!\n");
    }
    else
    {
        memset(buf->buf_vaddr, 0, tw);
        s_src[g_config.channel_front] = buf->buf_vaddr;
        s_src[g_config.channel_rear] = buf->buf_vaddr;
        s_src[g_config.channel_left] = buf->buf_vaddr;
        s_src[g_config.channel_right] = buf->buf_vaddr;
    }
    
#ifdef CALC_ALGO_TIME   
    struct timeval t_start;
    struct timeval t_end;
    unsigned int t_diff = 0;
    int fcnt = 0;
    
    gettimeofday(&t_start, NULL);
#endif

    setpriority(PRIO_PROCESS, 0, -20);

    init();
    
    DEBUG("gpu thread is ready");

    while (!g_exit)
    {
        if (s_ready == 0)
        {
	    pthread_mutex_lock(&stitch_2d_mutex);
	    pthread_cond_wait(&stitch_2d_conf, &stitch_2d_mutex);
	    pthread_mutex_unlock(&stitch_2d_mutex);
	}
	
	s_ready = 0;
	
	front.bufYUV = s_src[g_config.channel_front];
        rear.bufYUV = s_src[g_config.channel_rear];
        left.bufYUV = s_src[g_config.channel_left];
        right.bufYUV = s_src[g_config.channel_right];
        output.bufYUV = s_gpuFrameBuffer[texture].yVirt;
        
        doStitching(&front, &rear, &left, &right, &output);
        	
	render(texture);
	eglSwapBuffers(egldisplay, eglsurface);
	frames++;
	texture++;
	texture = texture % MAX_NUM_TEXELS;
	
#ifdef CALC_ALGO_TIME 	
        fcnt++;
	if(fcnt % 500 == 0)
	{
            gettimeofday(&t_end, NULL);
            t_diff = t_end.tv_sec - t_start.tv_sec;
            DEBUG("image end %d, time us:%ds fps:%d !\n",
		fcnt,
		t_diff,
		500/t_diff);
            gettimeofday(&t_start, NULL);
	}
#endif
    }

    DEBUG("gpu thread exit");
    return 0;
}

void stitch_2d_signal(int channel, unsigned long addr)
{    
    static int cnt = 0;
    
    s_src[channel] = addr;
    if (++cnt == g_config.camera_count)
    {
        cnt = 0;
        pthread_mutex_lock(&stitch_2d_mutex);
        s_ready = 1;
        pthread_cond_signal(&stitch_2d_conf);
        pthread_mutex_unlock(&stitch_2d_mutex);   
    }
}

void stitch_2d_exit()
{
    pthread_mutex_lock(&stitch_2d_mutex);
    pthread_cond_signal(&stitch_2d_conf);
    pthread_mutex_unlock(&stitch_2d_mutex);
}
