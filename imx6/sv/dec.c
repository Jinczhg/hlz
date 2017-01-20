/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-11-30
 * Author: ryan
 */

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "trace.h"
#include "avtp.h"
#include "dec.h"
#include "sv.h"
#include "gpu.h"
#include "ipu_test.h"

#define AVTP_STREAMID_BASE 0x3350

static decoder *decs[CHANNEL_NUM_MAX] = {0};
static int s_exit = 0;

extern sv_config g_config;

static int decode_fill_bs_buffer_from_net(decoder *dec, int default_size)
{
    RetCode ret;
    PhysicalAddress pa_read_ptr, pa_write_ptr;
    Uint32 target_addr, end_addr, space;
    int size = 0;
    int room_head = 0, room_tail = 0;

    frame *f = dec->frame_read;

again:
    if (!dec->frame_read)
    {
        pthread_mutex_lock(&dec->mutex);
	pthread_cond_wait(&dec->cond, &dec->mutex);
	dec->frame_reading= 1;
	pthread_mutex_unlock(&dec->mutex);
	if (!dec->frame_read)
	{
	    dec->frame_reading= 0;
	    ERROR("read frame should be NULL");
	    goto again;
	}
	//DEBUG("dec->frame_read len = %d", dec->frame_read->len);
    }
    
    f = dec->frame_read;

    ret = vpu_DecGetBitstreamBuffer(dec->handle, &pa_read_ptr, &pa_write_ptr, &space);

    if (ret != RETCODE_SUCCESS)
    {
        pthread_mutex_lock(&dec->mutex);
        dec->frame_read = NULL;
        dec->frame_reading= 0;
        pthread_mutex_unlock(&dec->mutex);
        
	ERROR("vpu_DecGetBitstreamBuffer failed");
	return -1;
    }
    
#if 0
    DEBUG("space=%lu, start_addr=%ld, end_addr=%ld,"
	    " pa_read_ptr=%ld, pa_write_ptr=%ld, frame_len=%d",
	    space, dec->phy_bs_buf_addr, dec->phy_bs_buf_addr + STREAM_BUF_SIZE,
	    pa_read_ptr ,pa_write_ptr, f->len);
#endif

    /* Decoder bitsteam buffer is full */
    if (space < f->len)
    {
        pthread_mutex_lock(&dec->mutex);
        dec->frame_read = NULL;
        dec->frame_reading= 0;
        pthread_mutex_unlock(&dec->mutex);
        
	DEBUG("space %lu < frame len %d", space, f->len);
	return 0;
    }

    size = f->len ;
    
    /* Fill the bitstream buffer */
    target_addr = dec->virt_bs_buf_addr + (pa_write_ptr - dec->phy_bs_buf_addr);
    end_addr = dec->virt_bs_buf_addr + STREAM_BUF_SIZE;
    if ((target_addr + size) > end_addr)
    {
	room_tail = end_addr - target_addr;
	room_head = size - room_tail;
    }
    else
    {
	room_tail = size;
	room_head = 0;
    }

    if (room_tail > 0)
    {
	memcpy((char*)target_addr, f->buf, room_tail);
	f->offset += room_tail;
    }

    target_addr = dec->virt_bs_buf_addr;
    if (room_head)
    {
	memcpy((char*)target_addr, f->buf + f->offset, room_head);
    }

    //release read frame
    pthread_mutex_lock(&dec->mutex);
    dec->frame_read = NULL;
    dec->frame_reading= 0;
    pthread_mutex_unlock(&dec->mutex);
    
    ret = vpu_DecUpdateBitstreamBuffer(dec->handle, size);
    if (ret != RETCODE_SUCCESS)
    {
	ERROR("vpu_DecUpdateBitstreamBuffer failed");
	return -1;
    }
    return size;
}

static int decode_fill_bs_buffer_from_file(decoder *dec, int default_size)
{
    RetCode ret;
    PhysicalAddress pa_read_ptr, pa_write_ptr;
    Uint32 target_addr, end_addr, space;
    int size = 0;
    int room_head = 0, room_tail = 0;
    static int fd = -1;
    int len = 0;

    ret = vpu_DecGetBitstreamBuffer(dec->handle, &pa_read_ptr, &pa_write_ptr, &space);

    if (ret != RETCODE_SUCCESS)
    {
	ERROR("vpu_DecGetBitstreamBuffer failed");
	return -1;
    }

    /* Decoder bitsteam buffer is full */
    if (space <= 0)
    {
	//DEBUG("space %lu <= 0", space);
	return 0;
    }

    if (default_size > 0)
    {
	if (space < default_size)
	{
	    return 0;
	}

	size = default_size;
    }
    else
    {
	size = ((space >> 9) << 9);
    }

    if (size == 0)
    {
	//DEBUG("size == 0, space %lu", space);
	return 0;
    }

    if (fd < 0)
    {
	fd = open("./data.h264", O_RDONLY);
        //fd = open("./data_0.h264", O_RDONLY);
	if (fd < 0)
	{
	    ERROR("open ./data.h264 error:%s", strerror(errno));
	    return -1;
	}
    }

    /* Fill the bitstream buffer */
    target_addr = dec->virt_bs_buf_addr + (pa_write_ptr - dec->phy_bs_buf_addr);
    end_addr = dec->virt_bs_buf_addr + STREAM_BUF_SIZE;
    if ((target_addr + size) > end_addr)
    {
	room_tail = end_addr - target_addr;
	room_head = size - room_tail;
    }
    else
    {
	room_tail = size;
	room_head = 0;
    }

    if (room_tail > 0)
    {
	if ((len = read(fd, (char*)target_addr, room_tail)) != room_tail)
	{
	    size = len;
	    close(fd);
	    fd = -1;
	}
    }

    target_addr = dec->virt_bs_buf_addr;
    if (room_head > 0 && len == room_tail)
    {
	if ((len = read(fd, (char*)target_addr, room_head)) != room_head)
	{
	    size = room_tail + len;
	    close(fd);
	    fd = -1;
	}
    }
    
    DEBUG("space=%lu, start_addr=%ld, end_addr=%ld,"
	    " pa_read_ptr=%ld, pa_write_ptr=%ld, frame_len=%d",
	    space, dec->phy_bs_buf_addr, dec->phy_bs_buf_addr + STREAM_BUF_SIZE,
	    pa_read_ptr ,pa_write_ptr, size);

    ret = vpu_DecUpdateBitstreamBuffer(dec->handle, size);
    if (ret != RETCODE_SUCCESS)
    {
	ERROR("vpu_DecUpdateBitstreamBuffer failed");
	return -1;
    }

    return size;
}

static int decode_fill_bs_buffer(decoder *dec, int default_size)
{
#if 0 
    return decode_fill_bs_buffer_from_file(dec, default_size);
#else
    return decode_fill_bs_buffer_from_net(dec, default_size);
#endif
}

static int decode_open(decoder *dec)
{
    RetCode ret;

    DecHandle handle = {0};
    DecOpenParam oparam = {0};

    if (g_config.mapType == LINEAR_FRAME_MAP)
    {
	g_config.tiled2LinearEnable = 0;
    }
    else
    {
	g_config.chromaInterleave = 1;
    }

    oparam.bitstreamFormat = g_config.format;
    oparam.bitstreamBuffer = dec->phy_bs_buf_addr;
    //oparam.bitstreamCPUAddr = dec->cpu_bs_buf_addr;
    oparam.bitstreamBufferSize = STREAM_BUF_SIZE;
    oparam.pBitStream = (uint8_t*)dec->virt_bs_buf_addr;
    oparam.reorderEnable = g_config.reorderEnable;
    oparam.mp4DeblkEnable = g_config.deblockEnable;
    oparam.chromaInterleave = g_config.chromaInterleave;
    oparam.mp4Class = g_config.mp4Class;
    if (cpu_is_mx6x())
    {
	oparam.avcExtension = g_config.mp4Class;
    }
    oparam.mjpg_thumbNailDecEnable = 0;
    oparam.mapType = g_config.mapType;
    oparam.tiled2LinearEnable = g_config.tiled2LinearEnable;
    oparam.bitstreamMode = g_config.bitstreamMode;
    oparam.jpgLineBufferMode = g_config.jpgLineBufferMode;
    oparam.psSaveBuffer = dec->phy_ps_buf_addr;
    oparam.psSaveBufferSize = PS_SAVE_SIZE;
    oparam.picWidth = 1280;
    oparam.picHeight = 720;

    ret = vpu_DecOpen(&handle, &oparam);
    if (ret != RETCODE_SUCCESS)
    {
	ERROR("vpu_DecOpen failed, ret:%d\n", ret);
	return -1;
    }

    dec->handle = handle;

    return 0;
}

static void decode_close(decoder *dec)
{
    RetCode ret;
    
    if (!dec->handle)
    {
	ret = vpu_DecClose(dec->handle);
	if (ret == RETCODE_FRAME_NOT_COMPLETE)
	{
	    vpu_SWReset(dec->handle, 0);
	    ret = vpu_DecClose(dec->handle);
	    if (ret != RETCODE_SUCCESS)
	    {
		ERROR("vpu_DecClose failed");
	    }
	}
    }
}

static int decode_parse(decoder *dec)
{
    DecInitialInfo initinfo = {0};
    DecHandle handle = dec->handle;
    RetCode ret = -1;
    int origWidth, origHeight;
    char *count;
    int extended_fbcount = 0;

#if 1
    while (decode_fill_bs_buffer(dec, 0) > 0)
    {
	vpu_DecSetEscSeqInit(handle, 1);
	ret = vpu_DecGetInitialInfo(handle, &initinfo);
	vpu_DecSetEscSeqInit(handle, 0);
	if (ret != RETCODE_SUCCESS)
	{
	    ERROR("vpu_DecGetInitialInfo failed, ret:%d, errorcode:%ld", ret, initinfo.errorcode);
	    //return -1;
	}
	else
	{
	    break;
	}
    }

    if (ret != RETCODE_SUCCESS)
    {
	return -1;
    }
#else
    while (decode_fill_bs_buffer(dec, 0) > 0);

    vpu_DecSetEscSeqInit(handle, 1);
    ret = vpu_DecGetInitialInfo(handle, &initinfo);
    vpu_DecSetEscSeqInit(handle, 0);
    if (ret != RETCODE_SUCCESS)
    {
	ERROR("vpu_DecGetInitialInfo failed, ret:%d, errorcode:%ld", ret, initinfo.errorcode);
	return -1;
    }
#endif
    if (initinfo.streamInfoObtained)
    {
	switch (g_config.format)
	{
	    case STD_AVC:
		DEBUG("H.264 Profile:%d, Level:%d, Interlace:%d",
			initinfo.profile, initinfo.level, initinfo.interlace);
		
		if (initinfo.aspectRateInfo)
		{
		    int aspect_ratio_idc;
		    int sar_width, sar_height;

		    if ((initinfo.aspectRateInfo >> 16) == 0)
		    {
			aspect_ratio_idc = initinfo.aspectRateInfo & 0xFF;
			DEBUG("aspect_ratio_idc: %d", aspect_ratio_idc);
		    }
		    else
		    {
			sar_width = (initinfo.aspectRateInfo >> 16) & 0xFFFF;
                        sar_height = initinfo.aspectRateInfo & 0xFFFF;
			DEBUG("sar_width: %d, sar_height: %d", sar_width, sar_height);
		    }
		}
		else
		{
		    DEBUG("Aspect Ratio is not present.");
		}

		break;

	    default:
		break;
	}
    }

    dec->last_pic_width = initinfo.picWidth;
    dec->last_pic_height = initinfo.picHeight;

    if (cpu_is_mx6x())
    {
	DEBUG("Decoder: width = %d, height = %d, frameRateRes = %lu, frameRateDiv = %lu, count = %u",
		initinfo.picWidth, initinfo.picHeight,
		initinfo.frameRateRes, initinfo.frameRateDiv,
		initinfo.minFrameBufferCount);
    }
    else
    {
	DEBUG("Decoder: width = %d, height = %d, fps = %lu, count = %u",
		initinfo.picWidth, initinfo.picHeight,
		initinfo.frameRateInfo,
		initinfo.minFrameBufferCount);
    }

    /*
     * We suggest to add two more buffers than minFrameBufferCount:
     *
     * vpu_DecClrDispFlag is used to control framebuffer whether can be
     * used for decoder again. One framebuffer dequeue from IPU is delayed
     * for performance improvement and one framebuffer is delayed for
     * display flag clear.
     *
     * Performance is better when more buffers are used if IPU performance
     * is bottleneck.
     *
     * Two more buffers may be needed for interlace stream from IPU DVI view
     */
    dec->min_fb_count = initinfo.minFrameBufferCount;
    count = getenv("VPU_EXTENDED_BUFFER_COUNT");

    if (count)
    {
	extended_fbcount = atoi(count);
    }
    else
    {
	extended_fbcount = 2;
    }

    if (initinfo.interlace)
    {
	dec->reg_fb_count = dec->min_fb_count + extended_fbcount + 2;
    }
    else
    {
	dec->reg_fb_count = dec->min_fb_count + extended_fbcount;
    }
    
    DEBUG("minfb=%d, extfb=%d, frameBufDelay=%d, nextDecodedIdxNum=%d", 
	    dec->min_fb_count, extended_fbcount, initinfo.frameBufDelay, initinfo.nextDecodedIdxNum);

    origWidth = initinfo.picWidth;
    origHeight = initinfo.picHeight;

    dec->pic_width = ((origWidth + 15) & ~15);
    if (initinfo.interlace == 1)
    {
       dec->pic_height = ((origHeight + 31) & ~31);
    }
    else
    {
        dec->pic_height = ((origHeight + 15) & ~15);
    }

    DEBUG("pic_width=%d, pic_height=%d, origWidth=%d, origHeight=%d", 
	    dec->pic_width, dec->pic_height, origWidth, origHeight);

    if (dec->pic_width == 0 || dec->pic_height == 0)
    {
	return -1;
    }

    /*
     * Information about H.264 decoder picture cropping rectangle which
     * presents the offset of top-left point and bottom-right point from
     * the origin of frame buffer.
     *
     * By using these four offset values, host application can easily
     * detect the position of target output window. When display cropping
     * is off, the cropping window size will be 0.
     *
     * This structure for cropping rectangles is only valid for H.264
     * decoder case.
     */

    /* Add non-h264 crop support, assume left=top=0 */
    if ((dec->pic_width > origWidth || dec->pic_height > origHeight)
	    && !initinfo.picCropRect.left && !initinfo.picCropRect.top
	    && !initinfo.picCropRect.right && !initinfo.picCropRect.bottom)
    {
	initinfo.picCropRect.left = 0;
	initinfo.picCropRect.top = 0;
	initinfo.picCropRect.right = origWidth;
	initinfo.picCropRect.bottom = origHeight;
    }

    DEBUG("CROP left=%lu, top=%lu, right=%lu, bottom=%lu",
	    initinfo.picCropRect.left,
	    initinfo.picCropRect.top,
	    initinfo.picCropRect.right,
	    initinfo.picCropRect.bottom);

    memcpy(&dec->pic_crop_rect, &initinfo.picCropRect, sizeof(initinfo.picCropRect));
    
    /* worstSliceSize is in kilo-byte uint */
    dec->phy_slice_buf_size = initinfo.worstSliceSize * 1024;
    dec->stride = dec->pic_width;

    /* Allocate memory for frame status, Mb and Mv report */
    if (dec->frame_buf_stat.enable)
    {
	dec->frame_buf_stat.addr = malloc(initinfo.reportBufSize.frameBufStatBufSize);
	if (!dec->frame_buf_stat.addr)
	{
	    ERROR("malloc error");
	}
    }
    if (dec->mb_info.enable)
    {
	dec->mb_info.addr = malloc(initinfo.reportBufSize.mbInfoBufSize);
	if (!dec->frame_buf_stat.addr)
	{
	    ERROR("malloc error");
	}
    }
    if (dec->mv_info.enable)
    {
	dec->mv_info.addr = malloc(initinfo.reportBufSize.mvInfoBufSize);
	if (!dec->frame_buf_stat.addr)
	{
	    ERROR("malloc error");
	}
    }

    return 0;
}

static int decode_allocate_framebuffer(decoder *dec)
{
   DecBufInfo bufinfo;
   DecHandle handle = dec->handle;
   frame_buffer *fb_pool = NULL;
   RetCode ret;
   int i = 0;
   int totalfb = 0, extrafb = IPU_TASK_NUM;
   int stride, height, divX, divY, size;
   int delay = -1;
   ipu_buffer *buf;

   dec->extra_fb_count = extrafb;
   totalfb = dec->reg_fb_count + extrafb;
   DEBUG("regfb = %d, extrafb = %d", dec->reg_fb_count, extrafb);

   stride = dec->stride;
   height = dec->pic_height;
   divX = 2;
   divY = 2;
   size = stride * height + stride / divX * height / divY + stride / divX * height / divY;

   fb_pool = &dec->fb_pool;
   
   fb_pool->fb = calloc(totalfb, sizeof(FrameBuffer));
   if (!fb_pool->fb)
   {
       ERROR("Failed to allocate fb");
       return -1;
   }

   fb_pool->mem_desc = malloc(totalfb * sizeof(vpu_mem_desc));
   if (!fb_pool->mem_desc)
   {
       ERROR("Failed to allocate mem_desc");
       return -1;
   }

   for (i = 0; i < totalfb; i++)
   {
       if (i < dec->reg_fb_count)
       {
       fb_pool->mem_desc[i].size = size;
       ret = IOGetPhyMem(&fb_pool->mem_desc[i]);
       if (ret)
       {
	   ERROR("Frame buffer allocation failed");
	   memset(&fb_pool->mem_desc[i], 0, sizeof(vpu_mem_desc));

	   return -1;
       }

       fb_pool->mem_desc[i].virt_uaddr = IOGetVirtMem(&(fb_pool->mem_desc[i]));

       fb_pool->fb[i].myIndex = i;
       fb_pool->fb[i].bufY = fb_pool->mem_desc[i].phy_addr;
       fb_pool->fb[i].bufCb = fb_pool->fb[i].bufY + stride * height;
       fb_pool->fb[i].bufCr = fb_pool->fb[i].bufCb + (stride / divX) * (height / divY);
       //fb_pool->fb[i].strideY = stride;
       //fb_pool->fb[i].strideC = stride / divX;
       fb_pool->fb[i].bufMvCol = fb_pool->fb[i].bufCr + stride / divX * height / divY;
       }
       else
       {
       buf = ipu_get_buffer(dec->channel, i - dec->reg_fb_count);
       fb_pool->mem_desc[i].size = size;

       fb_pool->mem_desc[i].phy_addr = buf->input_paddr;
       fb_pool->mem_desc[i].virt_uaddr = (unsigned long)buf->input_vaddr;

       fb_pool->fb[i].myIndex = i;
       fb_pool->fb[i].bufY = fb_pool->mem_desc[i].phy_addr;
       fb_pool->fb[i].bufCb = fb_pool->fb[i].bufY + stride * height;
       fb_pool->fb[i].bufCr = fb_pool->fb[i].bufCb + (stride / divX) * (height / divY);
       //fb_pool->fb[i].strideY = stride;
       //fb_pool->fb[i].strideC = stride / divX;
       fb_pool->fb[i].bufMvCol = fb_pool->fb[i].bufCr + stride / divX * height / divY;
       }
   }
   
   DEBUG("Frame buffer allocation OK");

   stride = (stride + 15) & ~15;

   if (g_config.format == STD_AVC)
   {
       bufinfo.avcSliceBufInfo.bufferBase = dec->phy_slice_buf_addr;
       bufinfo.avcSliceBufInfo.bufferSize = dec->phy_slice_buf_size;
   }

   // User needs to fill max supported macro block value of frame as following
   bufinfo.maxDecFrmInfo.maxMbX = stride / 16;
   bufinfo.maxDecFrmInfo.maxMbY = height / 16;
   bufinfo.maxDecFrmInfo.maxMbNum = stride * height / 256;

   /* For H.264, we can overwrite initial delay calculated from syntax
    * delay can be 0,1,...(in unit of frames)
    * Set to -1 or do not call this cammand if you don't want to overwrite it.
    * Take care not to set initial delay lower than reorder depth of the clip,
    * otherwise, display will be out of order.
    */
   vpu_DecGiveCommand(handle, DEC_SET_FRAME_DELAY, &delay);

   ret = vpu_DecRegisterFrameBuffer(handle, fb_pool->fb, dec->reg_fb_count, stride, &bufinfo);
   if (ret != RETCODE_SUCCESS)
   {
       ERROR("vpu_DecRegisterFrameBuffer failed, ret=%d", ret);
       return -1;
   }

   DEBUG("channel=%d: decode_allocate_framebuffer ok", dec->channel);

   return 0;
}

static int decode_start(decoder *dec)
{
    DEBUG("start......");
    DecHandle handle = dec->handle;
    DecOutputInfo outinfo = {0};
    DecParam decparam = {0};
    RetCode ret;
    FrameBuffer *fb = dec->fb_pool.fb;
    int decIndex = 0;
    int loop_id = 0;
    int frame_id = 0;
    int rot_id = 0;
    int rot_angle = 0;
    int rot_stride = dec->pic_width;
    int mirror = 0;
    int disp_clr_index = -1;
    ipu_buffer *buf;
    int num = 0;
    
    struct timeval t_start;
    struct timeval t_end;
    unsigned int t_diff = 0;

    decparam.dispReorderBuf = 0;
    decparam.skipframeMode = 0;
    decparam.skipframeNum = 0;
    /*
     * once iframeSearchEnable is enabled, prescanEnable, prescanMode
     * and skipframeMode options are ignored
     */
    decparam.iframeSearchEnable = 1;

    DEBUG("channel=%d", dec->channel);

    rot_id = dec->reg_fb_count;
   
#if 1
    vpu_DecGiveCommand(handle, SET_ROTATION_ANGLE, &rot_angle);
    vpu_DecGiveCommand(handle, SET_MIRROR_DIRECTION, &mirror);
    vpu_DecGiveCommand(handle, SET_ROTATOR_STRIDE, &rot_stride);
#endif

    vpu_DecBitBufferFlush(handle);
    
    gettimeofday(&t_start, NULL);

    while (!s_exit)
    {
#if 1
	vpu_DecGiveCommand(handle, SET_ROTATOR_OUTPUT, (void *)&fb[rot_id]);
	if (frame_id == 0)
	{
	    vpu_DecGiveCommand(handle, ENABLE_ROTATION, 0);
	    vpu_DecGiveCommand(handle, ENABLE_MIRRORING, 0);
	}
#endif	
	decode_fill_bs_buffer(dec, STREAM_FILL_SIZE);
	
	buf = ipu_get_buffer(dec->channel, num);
#if 1
	if (buf->write_lock)
	{
	    DEBUG("channel=%d: IPU thread not fast enough", dec->channel);
	    ipu_signal(dec->channel, dec->fb_pool.mem_desc[outinfo.indexFrameDisplay].virt_uaddr);
	    pthread_mutex_lock(&dec->ipu_mutex);
            pthread_cond_wait(&dec->ipu_cond, &dec->ipu_mutex);     
            pthread_mutex_unlock(&dec->ipu_mutex);
            if (buf->write_lock)
            {
                continue;
            }
	}
#endif
	ret = vpu_DecStartOneFrame(handle, &decparam);
	if (ret == RETCODE_JPEG_EOS)
	{
	    DEBUG("JPEG bitstram is end");
	    break;
	}
	else if (ret == RETCODE_JPEG_BIT_EMPTY)
	{
            continue;
	}
	else if (ret != RETCODE_SUCCESS)
	{
            ERROR("vpu_DecStartOneFrame failed, ret=%d", ret);
	    return -1;
	}

	loop_id = 0;
	while (vpu_IsBusy())
	{
	    /*
	     * Suppose vpu is hang if one frame cannot be decoded in 5s,
	     * then do vpu software reset.
	     * Please take care of this for network case since vpu
	     * interrupt also cannot be received if no enough data.
	     */
            if (loop_id == 50)
	    {
		ERROR("vpu_SWReset");
		ret = vpu_SWReset(handle, 0);
		return -1;
	    }

	    vpu_WaitForInt(100);

	    loop_id++;
	    //DEBUG("loop_id=%d", loop_id);
	}

	if (loop_id == 0)
	{
	    vpu_WaitForInt(100);
	}

	ret = vpu_DecGetOutputInfo(handle, &outinfo);

	usleep(0);

#if 0
        DEBUG("frame_id = %d, indexFrameDecoded = %d, indexFrameDisplay = %d, decodingSuccess = 0x%x.", 
		frame_id,
		outinfo.indexFrameDecoded,
		outinfo.indexFrameDisplay,
		outinfo.decodingSuccess);
#endif

	if (ret != RETCODE_SUCCESS)
	{
	    ERROR("vpu_DecGetOutputInfo failed, error code = %d, frame_id = %d", ret, frame_id);
	}

	if (outinfo.decodingSuccess == 0)
	{
	    DEBUG("Incomplete finish of decoding process, frame_id = %d", frame_id);
	    continue;
	}
        
	if (cpu_is_mx6x() && (outinfo.decodingSuccess & 0x10))
	{
	    DEBUG("channel = %d: vpu needs more bitstream in rollback mode, frame_id = %d", dec->channel, frame_id);

	    continue;
	}

	if (cpu_is_mx6x() && (outinfo.decodingSuccess & 0x100000))
	{
	    DEBUG("sequence parameters have been changed");
	}

	if (outinfo.notSufficientPsBuffer)
	{
	    ERROR("PS Buffer overflow");
	    return -1;
	}

	if (outinfo.notSufficientSliceBuffer)
	{
	    ERROR("Slice Buffer overflow");
	    return -1;
	}

	if (outinfo.indexFrameDisplay == -1)
	{
	    DEBUG("outinfo.indexFrameDisplay == -1");
	    break;
	}

	if (outinfo.indexFrameDisplay == -3 || outinfo.indexFrameDisplay == -2)
	{
            DEBUG("VPU doesn't hae picture to be displayed, indexFrameDisplay = %d",
		    outinfo.indexFrameDisplay);
	    continue;
	}
	
	if (outinfo.indexFrameDecoded >= 0)
	{
	    if (outinfo.decPicWidth != dec->last_pic_width 
		    || outinfo.decPicHeight != dec->last_pic_height)
	    {
                DEBUG("resolution chanhed from %dx%d to %dx%d",
			dec->last_pic_width, dec->last_pic_height,
			outinfo.decPicWidth, outinfo.decPicHeight);

		dec->last_pic_width = outinfo.decPicWidth;
		dec->last_pic_height = outinfo.decPicHeight;
	    }

	    decIndex++;
	}
        
	if (outinfo.indexFrameDisplay >= 0)
	{
	    if (dec->channel == g_config.channel_display)
            {
                //gpu_signal(dec->channel, dec->fb_pool.mem_desc[rot_id].virt_uaddr);
                gpu_signal(dec->channel, dec->fb_pool.mem_desc[outinfo.indexFrameDisplay].virt_uaddr);
            }
	        
	    buf->write_lock = 1;
	    buf->read_lock = 0;
	    ipu_signal(dec->channel, dec->fb_pool.mem_desc[outinfo.indexFrameDisplay].virt_uaddr);
	         
	    rot_id++;
	    if (rot_id == (dec->reg_fb_count + dec->extra_fb_count))
	    {
		rot_id = dec->reg_fb_count;
	    }
	        
	    num = rot_id - dec->reg_fb_count;
	    
	    disp_clr_index = outinfo.indexFrameDisplay + 1;
	    if (disp_clr_index == dec->reg_fb_count)
	    {
		disp_clr_index = 0;
	    }
	    vpu_DecClrDispFlag(handle, disp_clr_index);
	}

	frame_id++;
	if(frame_id % 500 == 0)
	{
            gettimeofday(&t_end, NULL);
            t_diff = t_end.tv_sec - t_start.tv_sec;
            DEBUG("channel = %d: image end %d, time us:%ds fps:%d !\n",
                dec->channel,
		frame_id,
		t_diff,
		500/t_diff);
            gettimeofday(&t_start, NULL);
	}
    }

    return 0;
}

int decode_init()
{
    int channel = 0;
    decoder *dec = NULL;
    int f = 0;

    DEBUG("init......");

    for (channel = 0; channel < CHANNEL_NUM_MAX; channel++)
    {
	decs[channel] = (decoder*)malloc(sizeof(decoder));
        dec = decs[channel];

	memset(dec, 0, sizeof(decoder));

	dec->channel = channel;

	for (f = 0; f < DEC_FRAME_NUM; f++)
	{
	    dec->frames[f].buf = malloc(DEC_FRAME_SIZE);
	    if (!dec->frames[f].buf)
	    {
		ERROR("malloc error: %s", strerror(errno));
		return -1;
	    }

	    dec->frames[f].offset = 0;
	    dec->frames[f].len = 0;
	    if (DEC_FRAME_NUM == f + 1)
	    {
		dec->frames[f].next =  &(dec->frames[0]);
	    }
	    else
	    {
		dec->frames[f].next =  &(dec->frames[f + 1]);
	    }
	}

	dec->frame_read = NULL;
	dec->frame_write = &(dec->frames[0]);

	pthread_mutex_init(&dec->mutex, NULL);
	pthread_cond_init(&dec->cond, NULL);
	
	pthread_mutex_init(&dec->ipu_mutex, NULL);
	pthread_cond_init(&dec->ipu_cond, NULL);
    }

    DEBUG("init OK");

    return 0;
}

void decode_deinit()
{
    int channel = 0;
    decoder *dec = NULL;
    int f = 0;
    
    DEBUG("deinit......");

    for (channel = 0; channel < CHANNEL_NUM_MAX; channel++)
    {
        dec = decs[channel];
	if (dec)
	{
	    pthread_mutex_destroy(&dec->mutex);
	    pthread_cond_destroy(&dec->cond);
	    
	    pthread_mutex_init(&dec->ipu_mutex, NULL);
	    pthread_cond_init(&dec->ipu_cond, NULL);

	    free(dec);
	    decs[channel] = NULL;
	}

	for (f = 0; f < DEC_FRAME_NUM; f++)
	{
            if (dec->frames[f].buf)
	    {
		free(dec->frames[f].buf);
		dec->frames[f].buf = NULL;
	    }
	}
    }

    DEBUG("deinit OK");
}

void* decode_thread(void *arg)
{
    int channel = *(int*)arg;
    decoder *dec = decs[channel];
    vpu_mem_desc bs_mem_desc = {0};
    vpu_mem_desc ps_mem_desc = {0};
    vpu_mem_desc slice_mem_desc = {0};
    int ret = 0;
    
    setpriority(PRIO_PROCESS, 0, -20);

    if (!dec)
    {
	DEBUG("need to call decode_init()");
	goto out;
    }
        
    bs_mem_desc.size = STREAM_BUF_SIZE;
    ret = IOGetPhyMem(&bs_mem_desc);
    if (ret)
    {
	ERROR("Unable to obtain physical mem");
	goto out;
    }

    if (IOGetVirtMem(&bs_mem_desc) <= 0)
    {
	ERROR("Unable to obtain virtual me");
	goto out;
    }
#if 0
    DEBUG("bs_mem_desc.phy_addr=0x%X, bs_mem_desc.cpu_addr=0x%X, bs_mem_desc.virt_uaddr=0x%X",
	    bs_mem_desc.phy_addr,
	    bs_mem_desc.cpu_addr,
	    bs_mem_desc.virt_uaddr);
#endif
    dec->phy_bs_buf_addr = bs_mem_desc.phy_addr;
    dec->cpu_bs_buf_addr = bs_mem_desc.cpu_addr;
    dec->virt_bs_buf_addr = bs_mem_desc.virt_uaddr;
    DEBUG("channel=%d, phy_bs_buf_addr=%lu", channel, dec->phy_bs_buf_addr);

    if (g_config.format == STD_AVC)
    {
	ps_mem_desc.size = PS_SAVE_SIZE;
	ret = IOGetPhyMem(&ps_mem_desc);
	if (ret)
	{
	    ERROR("Unable to obtain physical ps save mem");
	    goto out;
	}

	dec->phy_ps_buf_addr = ps_mem_desc.phy_addr;
    }

    ret = decode_open(dec);
    if (ret)
    {
	goto out;
    }

    ret = decode_parse(dec);
    if (ret)
    {
	ERROR("decoder pase failed");
	goto out;
    }

    if (g_config.format == STD_AVC)
    {
        slice_mem_desc.size = dec->phy_slice_buf_size;
	ret = IOGetPhyMem(&slice_mem_desc);
	if (ret)
	{
	    ERROR("Unable to obtain physical slice save mem");
	    goto out;
	}
	dec->phy_slice_buf_addr = slice_mem_desc.phy_addr;
    }

    ret = decode_allocate_framebuffer(dec);
    if (ret < 0)
    {
	ERROR("decode_allocate_frammebuffer failed");
	goto out;
    }

    g_config.camera_count++;
    
    decode_start(dec);

out:
    decode_close(dec);
    

    if (g_config.format == STD_AVC)
    {
	IOFreePhyMem(&slice_mem_desc);
	IOFreePhyMem(&ps_mem_desc);
    }

    IOFreeVirtMem(&bs_mem_desc);
    IOFreePhyMem(&bs_mem_desc);

    DEBUG("exit");
    
    return 0;
}

void decode_exit(int channel)
{
}

int decode_fill(int channel, uint8_t *data, int len)
{
    decoder *dec = decs[channel];
    frame *f = dec->frame_write;
    
    {
	if (data[0] == 0x0 && data[1] == 0x0 && data[2] == 0x0 && data[3] == 0x1
		&& (data[4] & 0x1F) == 0x7)
	{
	    //there is a finished image data
	    if (f->offset > 0)
	    {
		f->len = f->offset;
		f->offset = 0;

		pthread_mutex_lock(&(dec->mutex));
		if (dec->frame_reading == 0)
		{
		      dec->frame_read = f;
		      f = dec->frame_write = dec->frame_write->next;
		}
		pthread_cond_signal(&(dec->cond));
		pthread_mutex_unlock(&(dec->mutex));
	    }

	    f->offset = 0;
	    memcpy(f->buf + f->offset, data, len);
	    f->offset = len;
	}
	else if (f->offset > 0)
	{
	    memcpy(f->buf + f->offset, data, len);
	    f->offset += len;
	}
    }
    
    return 0;
}

void decode_reset(int channel)
{
    decoder *dec = decs[channel];

    dec->frame_write->offset = 0;
}

int decode_channel(int streamId)
{
    return (streamId - AVTP_STREAMID_BASE - 1) >> 1; //streamId: 0x3351, 0x3353, 0x3355, 0x3357
}

void decode_ipu_signal(int channel)
{
    decoder *dec = decs[channel];
    
    pthread_mutex_lock(&(dec->ipu_mutex));
    pthread_cond_signal(&(dec->ipu_cond));
    pthread_mutex_unlock(&(dec->ipu_mutex));
}
