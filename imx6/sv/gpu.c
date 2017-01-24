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

#include "gpu.h"
#include "dec.h"
#include "trace.h"
#include "sv.h"
#include "stitchalgo.h"

#define VERT_SHADER_FILE "./mxc_vgpu.vert" 
#define FRAG_SHADER_FILE "./mxc_vgpu.frag"


EGLNativeDisplayType native_display; 
EGLNativeWindowType  native_window; 
EGLDisplay egldisplay; 
EGLConfig eglconfig; 
EGLSurface eglsurface; 
EGLContext eglcontext; 

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
} nativeDisplay = {
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

#define MAX_NUM_TEXELS 2
GLuint texture[MAX_NUM_TEXELS];
GLvoid *pTexel[MAX_NUM_TEXELS][3];
static gpu_frame_buf s_gpuFrameBuffer[MAX_NUM_TEXELS];
GLint s_egl_texture[5] = {GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4};

unsigned int d8NumVerts = 6;

float d8Verts [] = {
  -1.0, -1.0, 0.0,
   1.0, -1.0, 0.0,
   1.0,  1.0, 0.0,
  -1.0, -1.0, 0.0,
   1.0,  1.0, 0.0,
  -1.0,  1.0, 0.0,
};

/* to resolve the mirror problem, frankli 20151028 */
float d8TexCoords [] = {
  1.0, 0.0,
  0.0, 0.0,
  0.0, 1.0,
  1.0, 0.0,
  0.0, 1.0,
  1.0, 1.0,
};

// Start with an identity matrix. 
GLfloat projectionMatrix[16] = 
{ 
    1.0f, 0.0f, 0.0f, 0.0f, 
    0.0f, 1.0f, 0.0f, 0.0f, 
    0.0f, 0.0f, 1.0f, 0.0f, 
    0.0f, 0.0f, 0.0f, 1.0f 
}; 

GLfloat transformMatrix[16] = 
{ 
    1.0f, 0.0f, 0.0f, 0.0f, 
    0.0f, 1.0f, 0.0f, 0.0f, 
    0.0f, 0.0f, 1.0f, 0.0f, 
    0.0f, 0.0f, 0.0f, 1.0f 
};  

static pthread_cond_t gpu_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gpu_mutex = PTHREAD_MUTEX_INITIALIZER;
static unsigned long s_src = 0;
static unsigned long s_output = 0;

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
	
	glTexDirectVIV(GL_TEXTURE_2D, SV_DISPLAY_WIDTH, SV_DISPLAY_HEIGHT, GL_VIV_NV12, (GLvoid **)&pTexel[i]);
	s_gpuFrameBuffer[i].yVirt = (unsigned int)pTexel[i][0]; 
	s_gpuFrameBuffer[i].cbVirt = (unsigned int)pTexel[i][1]; 
	s_gpuFrameBuffer[i].crVirt = 0;

        s_gpuFrameBuffer[i].width = SV_DISPLAY_WIDTH;  
	s_gpuFrameBuffer[i].height = SV_DISPLAY_HEIGHT;
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

    native_window  = fbCreateWindow(native_display, nativeDisplay.width / 2, 0,
	    nativeDisplay.width / 2, nativeDisplay.height); 
    DEBUG("eglCreateWindowSurface , the window width and height is %d:%d",
	    nativeDisplay.width / 2, nativeDisplay.height); 
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

int gpu_init()
{
    DEBUG("init...");
    //init();
    DEBUG("init OK");
    return 0;
}

void gpu_deinit()
{
    DEBUG("deinit OK");
}

void* gpu_thread(void *arg)
{
    int frames = 0;
    int texture = 0;
    
    int src_y_offset = (SV_IMAGE_WIDTH - SV_DISPLAY_WIDTH) / 2;
    int src_uv_offset = SV_IMAGE_WIDTH * SV_IMAGE_HEIGHT 
	+ (SV_IMAGE_WIDTH - SV_DISPLAY_WIDTH) / 2;

    int dst_uv_offset = SV_DISPLAY_WIDTH * SV_DISPLAY_HEIGHT;
    int y_h = SV_DISPLAY_HEIGHT;
    int uv_h = SV_DISPLAY_HEIGHT / 2;
    int h = 0;
    uint8_t *src = NULL;
    uint8_t *dst = NULL;
    uint8_t *tmp_src = NULL;
    
#ifdef CALC_ALGO_TIME   
    struct timeval t_start;
    struct timeval t_end;
    unsigned int t_diff = 0;
    int fcnt = 0;
    
    gettimeofday(&t_start, NULL);
#endif

    setpriority(PRIO_PROCESS, 0, -19);

    init();
    
    DEBUG("gpu thread is ready");

    while (!g_exit)
    {
	pthread_mutex_lock(&gpu_mutex);
	pthread_cond_wait(&gpu_cond, &gpu_mutex);    
	pthread_mutex_unlock(&gpu_mutex);
        
#if 1
        tmp_src = (uint8_t*)s_src;
	if (tmp_src == NULL)
	{
	   continue;
	}
	
	s_output = s_gpuFrameBuffer[texture].yVirt;
        if (s_output != 0 && tmp_src != 0)
	{
	    src = (uint8_t*)tmp_src + src_y_offset;
	    dst = (uint8_t*)s_output;
	    for (h = 0; h < y_h; h++)
	    {
		memcpy(dst, src, SV_DISPLAY_WIDTH);
		src += SV_IMAGE_WIDTH;
		dst += SV_DISPLAY_WIDTH;
	    }
            
	    src = (uint8_t*)tmp_src + src_uv_offset;
	    dst = (uint8_t*)s_output + dst_uv_offset;
	    for (h = 0; h < uv_h; h++)
	    {
		memcpy(dst, src, SV_DISPLAY_WIDTH);
		src += SV_IMAGE_WIDTH;
		dst += SV_DISPLAY_WIDTH;
	    }
	}
#endif	
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

void gpu_signal(int channel, unsigned long addr)
{
    pthread_mutex_lock(&gpu_mutex);
    s_src = addr;
    pthread_cond_signal(&gpu_cond);
    pthread_mutex_unlock(&gpu_mutex);
}

void gpu_exit()
{
    pthread_mutex_lock(&gpu_mutex);
    pthread_cond_signal(&gpu_cond);
    pthread_mutex_unlock(&gpu_mutex);
}
