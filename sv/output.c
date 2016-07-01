/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 */

#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>

#include "output.h"
#include "trace.h"

OutputBuffer* output_alloc(int cnt, int width, int height)
{
    int tw = 0;
    int32_t ret = 0;
    int32_t len = 0;
    uint64_t *vaddr;
    int64_t paddr = 0;
    OutputBuffer *head = NULL;
    OutputBuffer *tail = NULL;
    int cnt_done = 0;

    while (cnt)
    {
	OutputBuffer *buf = calloc(sizeof(OutputBuffer), 1);
	if (buf == NULL)
	{
	    ERROR("failed to calloc memory");
	    output_free(head, cnt_done);
	    return NULL;
	}

	tw = width * height * 3 / 2;

	ret  = SHM_alloc(tw, &buf->shmBuf);
	if (ret < 0)
	{
	    ERROR("failed to alloc shmem buffer");
	    free(buf);
	    output_free(head, cnt_done);
	    return NULL;
	}

        vaddr = (uint64_t *)buf->shmBuf.vir_addr;

	//make sure the memory is contiguous
	ret = mem_offset64(vaddr, NOFD, (size_t)tw, &paddr, (size_t *)&len);
        if (ret || len != tw)
	{
            ERROR("failed to check memory contigous ret %d errno %d len %d", ret, errno, len);
            SHM_release(&buf->shmBuf);
	    free(buf);
	    output_free(head, cnt_done);
	    return NULL;
	}

	buf->buf = (char *)vaddr;
	buf->y = (uint32_t)vaddr;
	buf->uv = (uint32_t)vaddr + width * height;

        buf->next = head;
	buf->tiler = false;
	buf->len = tw;
	head = buf;

	if (buf->next == NULL)
	{
	    tail = buf;
	}

	cnt--;
	cnt_done++;
    }

    if (tail)
    {
        tail->next = head;
    }

    return head;
}

void output_free(OutputBuffer *buf, int cnt)
{
    OutputBuffer *tmp = NULL;

    while (cnt)
    {
	tmp = buf;
	SHM_release(&tmp->shmBuf);
	buf = tmp->next;
	free(tmp);
	cnt--;
    }
}
