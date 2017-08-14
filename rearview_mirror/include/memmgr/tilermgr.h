/*
 *  Copyright 2012 by Texas Instruments Incorporated.
 *
 */

/*
 * tilermgr.h
 *
 * TILER library support functions for TI OMAP processors.
 *
 * Copyright (C) 2009-2010 Texas Instruments, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _TILERMGR_H_
#define _TILERMGR_H_

#include "mem_types.h"

#define TILERMGR_ERR_NONE (0)
#define TILERMGR_ERR_GENERIC (-1)

int TilerMgr_Open();
int TilerMgr_Close();
SSPtr TilerMgr_Alloc(enum pixel_fmt_t pixfmt, pixels_t width, pixels_t height);
int TilerMgr_Free(SSPtr ssptr);
SSPtr TilerMgr_PageModeAlloc(bytes_t length);
int TilerMgr_PageModeFree(SSPtr ssptr);
SSPtr TilerMgr_Map(void *ptr, bytes_t length);
int TilerMgr_Unmap(SSPtr ssptr);
SSPtr TilerMgr_VirtToPhys(void *ptr);

#endif
/*
 *  @(#) ti.sdo.tiler; 1, 0, 0,4; 2-22-2012 18:10:48; /db/atree/library/trees/fc/fc-q07/src/ xlibrary

 */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/tags/6.6.0/GA/lib/3rdparty/ti/ti_omx/framework_components_3_22_01_07/packages/ti/sdo/tiler/src/memmgr/tilermgr.h $ $Rev: 707218 $")
#endif
