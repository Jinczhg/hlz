/*
 *  Copyright 2012 by Texas Instruments Incorporated.
 *
 */

/*
 * tiler.h
 *
 * TILER driver support functions for TI OMAP processors.
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

#ifndef _TILER_H_
#define _TILER_H_

#define TILER_MEM_8BIT  0x60000000
#define TILER_MEM_16BIT 0x68000000
#define TILER_MEM_32BIT 0x70000000
#define TILER_MEM_PAGED 0x78000000
#define TILER_MEM_END   0x80000000

#define TILER_PAGE 0x1000
#define TILER_WIDTH    256
#define TILER_HEIGHT   128
#define TILER_BLOCK_WIDTH  64
#define TILER_BLOCK_HEIGHT 64
#define TILER_LENGTH (TILER_WIDTH * TILER_HEIGHT * TILER_PAGE)

#define TILER_DEVICE_PATH "/dev/tiler"
#define TILER_MAX_NUM_BLOCKS 16

enum tiler_fmt {
        TILFMT_MIN     = -2,
        TILFMT_INVALID = -2,
        TILFMT_NONE    = -1,
        TILFMT_8BIT    = 0,
        TILFMT_16BIT   = 1,
        TILFMT_32BIT   = 2,
        TILFMT_PAGE    = 3,
        TILFMT_MAX     = 3,
        TILFMT_8AND16  = 4              // removed ',' from original file
};

struct area {
        uint16_t width;
        uint16_t height;
};

struct tiler_block_info {
        enum tiler_fmt fmt;
        union {
                struct area area;
                uint32_t len;
        } dim;
        uint32_t stride;
        void *ptr;
        uint32_t id;
        uint32_t key;
        uint32_t group_id;
        /* alignment requirements for ssptr: ssptr & (align - 1) == offs */
        uint32_t align;
        uint32_t offs;
        uint32_t ssptr;
};

struct tiler_buf_info {
        uint32_t num_blocks;
        struct tiler_block_info blocks[TILER_MAX_NUM_BLOCKS];
        uint32_t offset;
        uint32_t length;
};

#define TILIOC_GBLK  _IOWR('z', 100, struct tiler_block_info)
#define TILIOC_FBLK   _IOW('z', 101, struct tiler_block_info)
#define TILIOC_GSSP  _IOWR('z', 102, uint32_t)
#define TILIOC_MBLK  _IOWR('z', 103, struct tiler_block_info)
#define TILIOC_UMBLK  _IOW('z', 104, struct tiler_block_info)
#define TILIOC_QBUF  _IOWR('z', 105, struct tiler_buf_info)
#define TILIOC_RBUF  _IOWR('z', 106, struct tiler_buf_info)
#define TILIOC_URBUF _IOWR('z', 107, struct tiler_buf_info)
#define TILIOC_QBLK  _IOWR('z', 108, struct tiler_block_info)
#define TILIOC_PRBLK  _IOW('z', 109, struct tiler_block_info)
#define TILIOC_URBLK  _IOW('z', 110, uint32_t)

#endif
/*
 *  @(#) ti.sdo.tiler; 1, 0, 0,4; 2-22-2012 18:10:48; /db/atree/library/trees/fc/fc-q07/src/ xlibrary

 */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/tags/6.6.0/GA/lib/3rdparty/ti/ti_omx/framework_components_3_22_01_07/packages/ti/sdo/tiler/src/memmgr/tiler.h $ $Rev: 707218 $")
#endif
