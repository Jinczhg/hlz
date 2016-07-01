/*
 * Copyright (c) 2013, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __LIBDCE_H__
#define __LIBDCE_H__

#include <xdc/std.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/video3/viddec3.h>
#include <ti/sdo/ce/video2/videnc2.h>
#include <ti/sdo/ce/video2/viddec2.h>

#if defined(BUILDOS_LINUX)
/* avoid some messy stuff in xdc/std.h which leads to gcc issues */
#define xdc__ARGTOPTR
#define xdc__ARGTOFXN
#endif /* BUILDOS_LINUX */

/********************************** DCE Error Types **********************************/
typedef enum dce_error_status {
    DCE_EOK = 0,
    DCE_EXDM_FAIL = -1,
    DCE_EOUT_OF_MEMORY = -2,
    DCE_EXDM_UNSUPPORTED = -3,
    DCE_EIPC_CREATE_FAIL = -4,
    DCE_EIPC_CALL_FAIL = -5,
    DCE_EINVALID_INPUT = -6,
    DCE_EOMAPDRM_FAIL = -7
} dce_error_status;

/***************************** Memory Allocation/Free APIs *****************************/
/*=====================================================================================*/
/** dce_alloc               : Allocate the Data structures passed to codec-engine APIs
 *                            except Input/Output buffers. This should be used for all
 *                            Parameter allocation needed for IPU remote proc.
 *
 * @ param sz    [in]       : Size of memory to be allocated.
 * @ return                 : Pointer to allocated memory.
 */
void *dce_alloc(int sz);

/*=====================================================================================*/
/** dsp_dce_alloc            : Allocate the Data structures passed to codec-engine APIs
 *                             except Input/Output buffers. This should be used for all
 *                             Parameter allocation needed for DSP remote proc.
 *
 * @ param sz    [in]       : Size of memory to be allocated.
 * @ return                 : Pointer to allocated memory.
 */
void *dsp_dce_alloc(int sz);

/*=====================================================================================*/
/** dce_free                : Free the Data structures passed to codec-engine APIs
 *                             except Input/Output buffers.
 *
 * @ param ptr   [in]       : Memory allocated by dce_alloc.
 */
void dce_free(void *ptr);

/*=====================================================================================*/
/** dsp_dce_free            : Free the memory
 *
 * @ param ptr   [in]       : memory allocated by dsp_dce_alloc .
 */
void dsp_dce_free(void *ptr);


/*********************************** APIs for Linux ***********************************/
/************************ Initialization/Deinitialization APIs ************************/
/*=====================================================================================*/
/** dce_init                : Initialize DCE. Only Linux applications are expected to call.
 *
 * @ return                 : Pointer to omap_device structure.
 */
void *dce_init(void);

/*===============================================================*/
/** dce_deinit          : Deinitialize DCE . Only Linux applications are expected to call.
 *
 * @ param dev    [in]      : Pointer to omap_device structure.
 */
void dce_deinit(void *dev);


/** dce_ipc_init            : Initialize DCE IPC. this is required to setup mmRpc link.
 *
 * @ return                 : Error code.
 */
int dce_ipc_init(int core);

/*===============================================================*/
/** dce_ipc_deinit          : Deinitialize DCE IPC.
 *
 * @ param dev    [in]      : Core ID and engine Table Idx, or -1 if no engine exists
 */
void dce_ipc_deinit(int core, int tableIdx);

/************************ Input/Output Buffer Lock/Unlock APIs ************************/
/*=====================================================================================*/
/** dce_buf_lock            : Pin or lock Tiler Buffers which would be used by the IVAHD codec
 *                            as reference buffers. API is specific to GLP.
 *
 * @ param num    [in]      : Number of buffers to be locked.
 * @ param handle [in]      : Pointer to array of DMA Buf FDs of the buffers to be locked.
 * @ return                 : DCE error status is returned.
 */
int dce_buf_lock(int num, size_t *handle);

/*=====================================================================================*/
/** dce_buf_unlock          : Unpin or unlock Tiler Buffers which were locked to be used
 *                            by the IVAHD codec as reference buffers. API is specific to GLP.
 *
 * @ param num    [in]      : Number of buffers to be locked.
 * @ param handle [in]      : Pointer to array of DMA Buf FDs of the buffers to be locked.
 * @ return                 : DCE error status is returned.
 */
int dce_buf_unlock(int num, size_t *handle);
/************************ Input/Output Buffer Lock/Unlock APIs ************************/
/*=====================================================================================*/
/** dsp_dce_buf_lock            : Pin or lock Tiler Buffers which would be used by the DSP codec
 *                            as reference buffers. API is specific to GLP.
 *
 * @ param num    [in]      : Number of buffers to be locked.
 * @ param handle [in]      : Pointer to array of DMA Buf FDs of the buffers to be locked.
 * @ return                 : DCE error status is returned.
 */
int dsp_dce_buf_lock(int num, size_t *handle);

/*=====================================================================================*/
/** dsp_dce_buf_unlock          : Unpin or unlock Tiler Buffers which were locked to be used
 *                            by the DSP codec as reference buffers. API is specific to GLP.
 *
 * @ param num    [in]      : Number of buffers to be locked.
 * @ param handle [in]      : Pointer to array of DMA Buf FDs of the buffers to be locked.
 * @ return                 : DCE error status is returned.
 */
int dsp_dce_buf_unlock(int num, size_t *handle);


/******************************* OMAPDRM Get/Set FD APIs *******************************/
/*=====================================================================================*/
/** dce_get_fd              : Get OMAP DRM File Descriptor.
 *
 * @ return                 : OMAP DRM File Descriptor.
 */
int dce_get_fd();

/*=====================================================================================*/
/** dce_set_fd              : Set OMAP DRM File Descriptor.
 *
 * @ param fd     [in]      : OMAP DRM File Descriptor.
 */
void dce_set_fd(int fd);


#endif /* __LIBDCE_H__ */

