/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Data: 2016-06-01
 * Author: ryan
 */

#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/time.h>

#include <xdc/std.h>
#include <ti/sdo/codecs/h264vdec/ih264vdec.h>
#include "dec.h"
#include "trace.h"
#include "codec_req.h"
#include "gpu.h"
#include "output.h"
#include "rvm.h"


static decode *decs[CHANNEL_NUM_MAX] = {0};

static int s_exit = 0;

static int s_padded_width = 0, s_padded_height = 0, s_y_offset = 0, s_uv_offset = 0;

int decode_padded_width()
{
    return s_padded_width;
}

int decode_padded_height()
{
    return s_padded_height;
}

int decode_y_offset()
{
    return s_y_offset;
}

int decode_uv_offset()
{
    return s_uv_offset;
}

int decode_init()
{
    int channel = 0;
    int i = 0;
    decode *dec = NULL;
    int width = ALIGN2(IMAGE_WIDTH, 4);
    int height = ALIGN2(IMAGE_HEIGHT, 4);
    s_padded_width = ALIGN2(width + (2 * PADX_H264), 7);
    s_padded_height = height + 4 * PADY_H264;
    int tw = s_padded_width * s_padded_height;
    int uv_topLeft = s_padded_width / 2;

    DEBUG("init......");

    s_y_offset = PADY_H264 * s_padded_width + PADX_H264;
    s_uv_offset = PADY_H264 * uv_topLeft + tw + PADX_H264;

    for (channel = 0; channel < CHANNEL_NUM_MAX; channel++)
    {
	decs[channel] = (decode*)malloc(sizeof(decode));
	dec = decs[channel];
	if (!dec)
	{
	    ERROR("channel = %d, malloc error: %s", channel, strerror(errno));

	    channel--;
	    for (; channel >= 0; channel--)
	    {
		free(decs[channel]);
	    }

	    return -1;
	}

        memset(dec, 0, sizeof(decode));
	
	dec->channel = channel;
	dec->width = width;
	dec->height = height;
	dec->padded_width = s_padded_width;
	dec->padded_height = s_padded_height;

	pthread_mutex_init(&dec->mutex, NULL);
	pthread_cond_init(&dec->cond, NULL);

	for (i = 0; i < DEC_BUFFER_NUM; i++)
	{
	    dec->bufs[i].buf = dce_alloc(DEC_BUFFER_LEN);
	    dec->bufs[i].offset = 0;
	    dec->bufs[i].write_lock = 0;
	    dec->bufs[i].read_lock = 1;
	    
	    dec->bufs[i].next = &dec->bufs[(i + 1) % DEC_BUFFER_NUM];
	}

	dec->buf = dec->bufs;
	dec->buf_count = 0;
    }

    DEBUG("init OK");
    return 0;
}

void decode_deinit()
{
    int channel = 0;
    decode *dec = NULL;
    int i = 0;

    for (channel = 0; channel < CHANNEL_NUM_MAX; channel++)
    {
	dec = decs[channel];
	if (dec)
	{
	    pthread_mutex_destroy(&dec->mutex);
	    pthread_cond_destroy(&dec->cond);
	    
	    for (i = 0; i < DEC_BUFFER_NUM; i++)
	    {
		if (dec->bufs[i].buf)
		{
		    dce_free(dec->bufs[i].buf);
		}
	    }

	    free(dec);
	    decs[channel] = NULL;
	}
    }

    DEBUG("deinit OK");
}

void* decode_thread(void *arg)
{
    CHANNEL channel = *(int*)arg;
    decode *dec = NULL;
    Engine_Error ee;
    XDAS_Int32 err;
    IH264VDEC_Params *h264_params = NULL;
    IH264VDEC_DynamicParams *h264_dynParams = NULL;
    IH264VDEC_Status *h264_status = NULL;

    int data = 0x01;
    //ThreadCtl(_NTO_TCTL_RUNMASK, &data);

    dec = decs[channel];

    // open engine
    dec->engine = Engine_open("ivahd_vidsvr", NULL, &ee);
    if (!dec->engine)
    {
	ERROR("channel = %d, could not open engine: %d", channel, ee);
	goto out;
    }

    DEBUG("channel = %d, Engine_open OK", channel);

    // params
    dec->params = dce_alloc(sizeof(IH264VDEC_Params));
    if (!dec->params)
    {
	ERROR("channel = %d, dce_alloc fail", channel);
	goto out;
    }
    dec->params->size = sizeof(IH264VDEC_Params);
    dec->params->maxWidth = dec->width;
    dec->params->maxHeight = dec->height;
    dec->params->maxFrameRate = 30000; // 30 fps
    dec->params->maxBitRate = 10000000; // 10 Mbps
    dec->params->dataEndianness = XDM_BYTE;
    dec->params->forceChromaFormat = XDM_YUV_420SP;
    dec->params->operatingMode = IVIDEO_DECODE_ONLY;
    dec->params->displayDelay = IVIDDEC3_DISPLAY_DELAY_AUTO;
    dec->params->displayBufsMode = IVIDDEC3_DISPLAYBUFS_EMBEDDED;
    dec->params->inputDataMode = IVIDEO_ENTIREFRAME;
    dec->params->metadataType[0] = IVIDEO_METADATAPLANE_NONE;
    dec->params->metadataType[1] = IVIDEO_METADATAPLANE_NONE;
    dec->params->metadataType[2] = IVIDEO_METADATAPLANE_NONE;
    dec->params->numInputDataUnits = 0;
    dec->params->outputDataMode = IVIDEO_ENTIREFRAME;
    dec->params->numOutputDataUnits = 0;
    dec->params->errorInfoMode = IVIDEO_ERRORINFO_OFF;

    h264_params = (IH264VDEC_Params*)dec->params;
    h264_params->dpbSizeInFrames = IH264VDEC_DPB_NUMFRAMES_AUTO;
    h264_params->pConstantMemory = 0;
    h264_params->presetLevelIdc = IH264VDEC_LEVEL41;
    h264_params->errConcealmentMode = IH264VDEC_APPLY_CONCEALMENT;
    h264_params->temporalDirModePred = TRUE;
    h264_params->detectCabacAlignErr = IH264VDEC_DISABLE_CABACALIGNERR_DETECTION;

    err = msync((Ptr)h264_params, sizeof(IH264VDEC_Params), MS_CACHE_ONLY | MS_SYNC);

    //dynParams
    dec->dynParams = dce_alloc(sizeof(IH264VDEC_DynamicParams));
    if (!dec->dynParams)
    {
	ERROR("channel = %d, dce_alloc fail", channel);
	goto out;
    }
    dec->dynParams->size = sizeof(IH264VDEC_DynamicParams);
    dec->dynParams->decodeHeader = XDM_DECODE_AU;
    dec->dynParams->displayWidth = 0;
    dec->dynParams->frameSkipMode = IVIDEO_NO_SKIP;
    dec->dynParams->newFrameFlag = XDAS_TRUE;
    h264_dynParams = (IH264VDEC_DynamicParams*)dec->dynParams;

    // status
    dec->status = dce_alloc(sizeof(IH264VDEC_Status));
    if (!dec->status)
    {
	ERROR("channnel = %d, dce_alloc fail", channel);
	goto out;
    }
    dec->status->size = sizeof(IH264VDEC_Status);
    h264_status = (IH264VDEC_Status*)dec->status;

    // codec
    dec->codec = VIDDEC3_create(dec->engine, "ivahd_h264dec", (VIDDEC3_Params*)h264_params);
    if (!dec->codec)
    {
        ERROR("channel = %d, VIDDEC3_create fail", channel);
        system("echo quit::1 >> /pps/hinge-tech/camera");
	goto out;
    }

    // control
    err = VIDDEC3_control(dec->codec, XDM_SETPARAMS, (VIDDEC3_DynamicParams*)h264_dynParams,
	    (VIDDEC3_Status*)h264_status);
    if (err)
    {
	ERROR("channel = %d, VIDDEC3_control XDM_SETPARAMS error: %d", channel, err);
	goto out;
    }

    // inBufs
    dec->inBufs = dce_alloc(sizeof(XDM2_BufDesc));
    if (!dec->inBufs)
    {
	ERROR("channel = %d, dce_alloc fail", channel);
	goto out;
    }
    dec->inBufs->numBufs = 1;
    dec->inBufs->descs[0].memType = XDM_MEMTYPE_RAW;

    // outBufs
    dec->outBufs = dce_alloc(sizeof(XDM2_BufDesc));
    if (!dec->outBufs)
    {
	ERROR("channel = %d, dce_alloc fail", channel);
	goto out;
    }
    dec->outBufs->numBufs = 2;
    dec->outBufs->descs[0].memType = XDM_MEMTYPE_RAW;
    dec->outBufs->descs[0].bufSize.bytes = dec->padded_width * dec->padded_height;
    dec->outBufs->descs[1].memType = XDM_MEMTYPE_RAW;
    dec->outBufs->descs[1].bufSize.bytes = dec->padded_width * dec->padded_height / 2;

    // inArgs
    dec->inArgs = dce_alloc(sizeof(IVIDDEC3_InArgs));
    if (!dec->inArgs)
    {
	ERROR("channel = %d, dce_alloc fail", channel);
	goto out;
    }
    dec->inArgs->size = sizeof(IVIDDEC3_InArgs);

    // outArgs
    dec->outArgs = dce_alloc(sizeof(IVIDDEC3_OutArgs));
    if (!dec->outArgs)
    {
	ERROR("channel = %d, dce_alloc fail", channel);
	goto out;
    }
    dec->outArgs->size = sizeof(IVIDDEC3_OutArgs);

    // start to decode
    decode_start(dec);

    DEBUG("end to decode, channel = %d\n", channel);

out:
    if (dec->params)
    {
	dce_free(dec->params);
    }

    if (dec->dynParams)
    {
	dce_free(dec->dynParams);
    }

    if (dec->status)
    {
	dce_free(dec->status);
    }

    if (dec->inBufs)
    {
	dce_free(dec->inBufs);
    }

    if (dec->outBufs)
    {
	dce_free(dec->outBufs);
    }

    if (dec->inArgs)
    {
	dce_free(dec->inArgs);
    }

    if (dec->outArgs)
    {
	dce_free(dec->outArgs);
    }

    if (dec->codec)
    {
	VIDDEC3_delete(dec->codec);
    }

    if (dec->engine)
    {
	Engine_close(dec->engine);
    }

    DEBUG("dec thread exit, channel = %d", channel);
    return 0;
}

int decode_start(decode *dec)
{
    OutputBuffer *output = NULL;
    XDAS_Int32 err;
    int yoff = 0;
    int uvoff = 0;
    int tw = dec->padded_width * dec->padded_height;
    int uv_topLeft = dec->padded_width / 2;
    dec_buffer *data = dec->bufs;

    yoff = PADY_H264 * dec->padded_width + PADX_H264;
    uvoff = PADY_H264 * uv_topLeft + tw + PADX_H264;
   
    DEBUG("channel = %d, start to decode", dec->channel);

    while (!s_exit)
    {
	if (data->read_lock)
	{
	    pthread_mutex_lock(&dec->mutex);
	    pthread_cond_wait(&dec->cond, &dec->mutex);
	    pthread_mutex_unlock(&dec->mutex);
	}

	if (s_exit)
	{
	    break;
	}
	
	OutputBuffer *output = gpu_get_buffer(dec->channel);
	dec->inArgs->inputID = (XDAS_Int32)output;
        dec->outBufs->descs[0].buf = (XDAS_Int8*)output->y;
        dec->outBufs->descs[1].buf = (XDAS_Int8*)output->uv;

	INFO("channel = %d, H.264 frame come", dec->channel);
	
	dec->inBufs->descs[0].buf = data->buf;
	dec->inBufs->descs[0].bufSize.bytes = data->len;
	dec->inArgs->numBytes = data->len;

	err = VIDDEC3_process(dec->codec, dec->inBufs, dec->outBufs, dec->inArgs, dec->outArgs);

	//release the buf first
	//data->len = 0;
	data->read_lock = 1;
	data->write_lock = 0;
	data = data->next;

	if (err == DCE_EXDM_FAIL)
	{
	    ERROR("channel = %d, VIDDEC3_process error: %d, outArgs->extendedError: %d",
		    dec->channel, err, dec->outArgs->extendedError);
	    if (XDM_ISFATALERROR(dec->outArgs->extendedError))
	    {
		ERROR("channel = %d, XDM_ISFATALERROR", dec->channel);
		break;
	    }

	    continue;
	}
	else
	{
	    INFO("channel = %d, VIDDEC3_process OK", dec->channel);
	}
	  
	gpu_signal(dec->channel, output);
    }

    return 0;
}

int decode_fill(int channel, const uint8_t *data, int len)
{
    decode *dec = decs[channel];
    dec_buffer *buf = dec->buf;

    if (data[0] == 0x0 && data[1] == 0x0 && data[2] == 0x0 && data[3] == 0x1
	    && (data[4] & 0x1F) == 0x7)
    {
	if (buf->offset > 0)
	{
	    INFO("frame len = %d", buf->offset);
	    
	    buf->len = buf->offset;
	    dec->buf_count++;
	    buf->offset = 0;
            buf->write_lock = 1;
	    buf->read_lock = 0;
	    
	    pthread_mutex_lock(&dec->mutex);
	    pthread_cond_signal(&dec->cond);
	    pthread_mutex_unlock(&dec->mutex);
       
	    buf = dec->buf->next;
	    dec->buf = buf;
	}

	if (!buf->write_lock)
	{
            memcpy(buf->buf, data, len);
	    buf->offset += len;
	}
	else
	{
	    DEBUG("channel = %d, decoding is  not fast enough, miss this frame", channel);
	}
    }
    else if (buf->offset > 0)
    {
	memcpy(buf->buf + buf->offset, data, len);
	buf->offset += len;

	if (len != 992) //992 is max payload len
	{
            INFO("frame len = %d", buf->offset);

	    buf->len = buf->offset;
	    dec->buf_count++;
	    buf->offset = 0;
            buf->write_lock = 1;
	    buf->read_lock = 0;

	    pthread_mutex_lock(&dec->mutex);
	    pthread_cond_signal(&dec->cond);
	    pthread_mutex_unlock(&dec->mutex);
       
	    buf = dec->buf->next;
	    dec->buf = buf;
	}
    }

    return 0;
}

void decode_reset(int channel)
{
    decode *dec = decs[channel];

    dec->buf->offset = 0;
}


void decode_output(uint8_t *yuv, uint8_t *y, uint8_t *uv, int width, int height, int stride)
{
    int i = 0;
    
    for (i = 0; i < height; i++)
    {
	memcpy(yuv, y, width);
	yuv += width;
	y += stride;
    }

    for (i = 0; i < height / 2; i++)
    {
	memcpy(yuv, uv, width);
	yuv += width;
	uv += stride;
    }
}

void decode_exit(int channel)
{
    decode *dec = decs[channel];
    s_exit = 1;

    pthread_mutex_lock(&dec->mutex);
    pthread_cond_signal(&dec->cond);
    pthread_mutex_unlock(&dec->mutex);
}
