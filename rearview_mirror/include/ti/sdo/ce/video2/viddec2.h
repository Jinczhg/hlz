/*
 *  Copyright 2013 by Texas Instruments Incorporated.
 *
 */

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
 *
 */
/*
 *  ======== viddec2.h ========
 */
/**
 *  @file       ti/sdo/ce/video2/viddec2.h
 *
 *  @brief      The VIDDEC2 video decoder interface.  Provides the user an
 *              interface to create and interact with XDAIS algorithms that are
 *              compliant with the XDM-defined IVIDDEC2 video decoder
 *              interface.
 */
/**
 *  @defgroup   ti_sdo_ce_video2_VIDDEC2    VIDDEC2 - Video Decoder Interface
 *
 *  This is the VIDDEC2 video decoder interface.  Several of the data
 *  types in this API are specified by the XDM interface; please see
 *  the XDM documentation for those details.
 */

#ifndef ti_sdo_ce_video2_VIDDEC2_
#define ti_sdo_ce_video2_VIDDEC2_

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/xdais/dm/xdm.h>
#include <ti/xdais/dm/ividdec2.h>

#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/visa.h>
#include <ti/sdo/ce/skel.h>

/** @ingroup    ti_sdo_ce_video2_VIDDEC2 */
/*@{*/

typedef IVIDDEC2_Status VIDDEC2_Status;       /**< @copydoc IVIDDEC2_Status */

#define VIDDEC2_EOK      IVIDDEC2_EOK         /**< @copydoc IVIDDEC2_EOK */
#define VIDDEC2_EFAIL    IVIDDEC2_EFAIL       /**< @copydoc IVIDDEC2_EFAIL */

/**< @copydoc IVIDDEC2_EUNSUPPORTED */
#define VIDDEC2_EUNSUPPORTED IVIDDEC2_EUNSUPPORTED

#define VIDDEC2_ETIMEOUT VISA_ETIMEOUT        /**< @copydoc VISA_ETIMEOUT */
#define VIDDEC2_FOREVER  VISA_FOREVER         /**< @copydoc VISA_FOREVER */

/**
 *  @brief      The VISA type
 */
#define VIDDEC2_VISATYPE "ti.sdo.ce.video2.IVIDDEC2"

/**
 *  @brief      Name of stub functions. Use this name when registering the
 *              VIDDEC2_STUBS functions with Engine_addStubFxns.
 *
 *  @sa         Engine_addStubFxns
 */
#define VIDDEC2_STUBSNAME "VIDDEC2_STUBS"


/**
 *  @brief      Opaque handle to a VIDDEC2 codec.
 */
typedef VISA_Handle VIDDEC2_Handle;

/**
 *  @brief      This structure defines the parameters necessary to create an
 *              instance of a video decoder object.
 */
typedef struct IVIDDEC2_Params VIDDEC2_Params;

/**
 * @copydoc IVIDDEC2_InArgs
 */
typedef IVIDDEC2_InArgs          VIDDEC2_InArgs;

/**
 * @copydoc IVIDDEC2_OutArgs
 */
typedef IVIDDEC2_OutArgs         VIDDEC2_OutArgs;

/**
 * @copydoc IVIDDEC2_Cmd
 */
typedef IVIDDEC2_Cmd             VIDDEC2_Cmd;

/**
 * @copydoc IVIDDEC2_DynamicParams
 */
typedef IVIDDEC2_DynamicParams   VIDDEC2_DynamicParams;

/** @cond INTERNAL */

/**
 *  @brief      An implementation of the skel interface; the skeleton side
 *              of the stubs.
 */
extern SKEL_Fxns VIDDEC2_SKEL;

/**
 *  @brief      Implementation of the IVIDDEC2 interface that is run remotely.
 */
extern IVIDDEC2_Fxns VIDDEC2_STUBS;

/** @endcond */

/**
 *  @brief      Definition of IVIDDEC2 codec class configurable parameters
 *
 *  @sa         VISA_getCodecClassConfig()
 */
typedef struct IVIDDEC2_CodecClassConfig {
    Bool manageInBufsCache     [ XDM_MAX_IO_BUFFERS ];
    Bool manageOutBufsCache    [ XDM_MAX_IO_BUFFERS ];
} IVIDDEC2_CodecClassConfig;


/*
 *  ======== VIDDEC2_create ========
 */
/**
 *  @brief      Create an instance of a video decoder algorithm.
 *
 *  Instance handles must not be concurrently accessed by multiple threads;
 *  each thread must either obtain its own handle (via VIDDEC2_create()) or
 *  explicitly serialize access to a shared handle.
 *
 *  @param[in]  e       Handle to an opened engine.
 *  @param[in]  name    String identifier of the type of video decoder
 *                      to create.
 *  @param[in]  params  Creation parameters.
 *
 *  @retval     NULL            An error has occurred.
 *  @retval     non-NULL        The handle to the newly created video decoder
 *                              instance.
 *
 *  @remarks    @c params is optional.  If it's not supplied, codec-specific
 *              default params will be used.
 *
 *  @remark     Depending on the configuration of the engine opened, this
 *              call may create a local or remote instance of the video
 *              decoder.
 *
 *  @codecNameRemark
 *
 *  @sa         Engine_open()
 *  @sa         VIDDEC2_delete()
 */
extern VIDDEC2_Handle VIDDEC2_create(Engine_Handle e, String name,
    VIDDEC2_Params *params);


/*
 *  ======== VIDDEC2_process ========
 */
/**
 *  @brief      Execute the process() method in this instance of a video
 *              decoder algorithm.
 *
 *  @param[in]  handle  Handle to a created video decoder instance.
 *  @param[in]  inBufs  A buffer descriptor containing input buffers.
 *  @param[out] outBufs A buffer descriptor containing output buffers.
 *  @param[in]  inArgs  Input Arguments.
 *  @param[out] outArgs Output Arguments.
 *
 *  @pre        @c handle is a valid (non-NULL) video decoder handle
 *              and the video decoder is in the created state.
 *
 *  @retval     #VIDDEC2_EOK         Success.
 *  @retval     #VIDDEC2_EFAIL       Failure.
 *  @retval     #VIDDEC2_EUNSUPPORTED Unsupported request.
 *
 *  @remark     Since the VIDDEC2 decoder contains support for asynchronous
 *              buffer submission and retrieval, this API becomes known as
 *              synchronous in nature.
 *
 *  @remark     This is a blocking call, and will return after the data
 *              has been decoded.
 *
 *  @remark     The buffers supplied to VIDDEC2_process() may have constraints
 *              put on them.  For example, in dual-processor, shared memory
 *              architectures, where the codec is running on a remote
 *              processor, the buffers may need to be physically contiguous.
 *              Additionally, the remote processor may place restrictions on
 *              buffer alignment.
 *
 *  @sa         VIDDEC2_create()
 *  @sa         VIDDEC2_delete()
 *  @sa         VIDDEC2_control()
 *  @sa         VIDDEC2_processAsync()
 *  @sa         VIDDEC2_processWait()
 *  @sa         IVIDDEC2_Fxns::process() - the reflected algorithm interface,
 *                                         which may contain further usage
 *                                         details.
 */
extern Int32 VIDDEC2_process(VIDDEC2_Handle handle, XDM1_BufDesc *inBufs,
    XDM_BufDesc *outBufs, VIDDEC2_InArgs *inArgs, VIDDEC2_OutArgs *outArgs);


/*
 *  ======== VIDDEC2_control ========
 */
/**
 *  @brief      Execute the control() method in this instance of a video
 *              decoder algorithm.
 *
 *  @param[in]  handle  Handle to a created video decoder instance.
 *  @param[in]  id      Command id for xDM control operation.
 *  @param[in]  params  Runtime control parameters used for decoding.
 *  @param[out] status  Status info upon completion of decode operation.
 *
 *  @pre        @c handle is a valid (non-NULL) video decoder handle
 *              and the video decoder is in the created state.
 *
 *  @retval     #VIDDEC2_EOK         Success.
 *  @retval     #VIDDEC2_EFAIL       Failure.
 *  @retval     #VIDDEC2_EUNSUPPORTED Unsupported request.
 *
 *  @remark     This is a blocking call, and will return after the control
 *              command has been executed.
 *
 *  @remark     If an error is returned, @c status->extendedError may
 *              indicate further details about the error.  See
 *              #VIDDEC2_Status::extendedError for details.
 *
 *  @sa         VIDDEC2_create()
 *  @sa         VIDDEC2_delete()
 *  @sa         IVIDDEC2_Fxns::process()
 */
extern Int32 VIDDEC2_control(VIDDEC2_Handle handle, VIDDEC2_Cmd id,
    VIDDEC2_DynamicParams *params, VIDDEC2_Status *status);


/*
 *  ======== VIDDEC2_delete ========
 */
/**
 *  @brief      Delete the instance of a video decoder algorithm.
 *
 *  @param[in]  handle  Handle to a created video decoder instance.
 *
 *  @remark     Depending on the configuration of the engine opened, this
 *              call may delete a local or remote instance of the video
 *              decoder.
 *
 *  @pre        @c handle is a valid (non-NULL) handle which is
 *              in the created state.
 *
 *  @post       All resources allocated as part of the VIDDEC2_create()
 *              operation (memory, DMA channels, etc.) are freed.
 *
 *  @sa         VIDDEC2_create()
 */
extern Void VIDDEC2_delete(VIDDEC2_Handle handle);


/*
 *  ======== VIDDEC2_processAsync ========
 */
/**
 *  @brief      Perform asynchronous submission to this instance of a video
 *              decoder algorithm.
 *
 *  @param[in]  handle  Handle to a created video decoder instance.
 *  @param[in]  inBufs  A buffer descriptor containing input buffers.
 *  @param[out] outBufs A buffer descriptor containing output buffers.
 *  @param[in]  inArgs  Input Arguments.
 *  @param[out] outArgs Output Arguments.
 *
 *  @pre        @c handle is a valid (non-NULL) video decoder handle
 *              and the video decoder is in the created state.
 *
 *  @retval     #VIDDEC2_EOK         Success.
 *  @retval     #VIDDEC2_EFAIL       Failure.
 *  @retval     #VIDDEC2_EUNSUPPORTED Unsupported request.
 *
 *  @remark     This API is the asynchronous counterpart to the process()
 *              method.  It allows for buffer and argument submission without
 *              waiting for retrieval.  A response is retrieved using the
 *              VIDDEC2_processWait() API.
 *
 *  @remark     The buffers supplied to VIDDEC2_processAsync() may have
 *              constraints put on them.  For example, in dual-processor,
 *              shared memory architectures, where the codec is running on a
 *              remote processor, the buffers may need to be physically
 *              contiguous.  Additionally, the remote processor may place
 *              restrictions on buffer alignment.
 *
 *  @sa         VIDDEC2_create()
 *  @sa         VIDDEC2_delete()
 *  @sa         VIDDEC2_control()
 *  @sa         VIDDEC2_process()
 *  @sa         VIDDEC2_processWait()
 *  @sa         IVIDDEC2_Fxns::process()
 */
extern XDAS_Int32 VIDDEC2_processAsync(VIDDEC2_Handle handle,
    XDM1_BufDesc *inBufs, XDM_BufDesc *outBufs,
    VIDDEC2_InArgs *inArgs, VIDDEC2_OutArgs *outArgs);


/*
 *  ======== VIDDEC2_processWait ========
 */
/**
 *  @brief      Wait for a return message from a previous invocation of
 *              VIDDEC2_processAsync() in this instance of an video decoder
 *              algorithm.
 *
 *  @param[in]  handle  Handle to a created video decoder instance.
 *  @param[in]  inBufs  A buffer descriptor containing input buffers.
 *  @param[out] outBufs A buffer descriptor containing output buffers.
 *  @param[in]  inArgs  Input Arguments.
 *  @param[out] outArgs Output Arguments.
 *  @param[in]  timeout Amount of "time" to wait (from 0 -> VIDDEC2_FOREVER)
 *
 *  @pre        @c handle is a valid (non-NULL) video decoder handle
 *              and the video decoder is in the created state.
 *
 *  @retval     #VIDDEC2_EOK         Success.
 *  @retval     #VIDDEC2_EFAIL       Failure.
 *  @retval     #VIDDEC2_EUNSUPPORTED Unsupported request.
 *  @retval     #VIDDEC2_ETIMEOUT    Operation timed out.
 *
 *  @remark     This is a blocking call, and will return after the data
 *              has been decoded.
 *
 *  @remark     "Polling" is supported by using a timeout of 0.  Waiting
 *              forever is supported by using a timeout of VIDDEC2_EFOREVER.
 *
 *  @remark     There must have previously been an invocation of the
 *              VIDDEC2_processAsync() API.
 *
 *  @remark     The buffers supplied to VIDDEC2_processAsync() may have
 *              constraints put on them.  For example, in dual-processor,
 *              shared memory architectures, where the codec is running on a
 *              remote processor, the buffers may need to be physically
 *              contiguous.  Additionally, the remote processor may place
 *              restrictions on buffer alignment.
 *
 *  @sa         VIDDEC2_create()
 *  @sa         VIDDEC2_delete()
 *  @sa         VIDDEC2_control()
 *  @sa         VIDDEC2_process()
 *  @sa         VIDDEC2_processAsync()
 */
extern XDAS_Int32 VIDDEC2_processWait(VIDDEC2_Handle handle,
    XDM1_BufDesc *inBufs, XDM_BufDesc *outBufs,
    VIDDEC2_InArgs *inArgs, VIDDEC2_OutArgs *outArgs, UInt timeout);


/*@}*/

#ifdef __cplusplus
}
#endif

#endif
/*
 *  @(#) ti.sdo.ce.video2; 1, 0, 3,3; 6-13-2013 00:20:38; /db/atree/library/trees/ce/ce-w08/src/ xlibrary

 */
