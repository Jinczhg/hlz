/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-06-01
 * Author: ryan
 */

#ifndef __TRACE__H
#define __TRACE__H

#include <stdio.h>

#define ERROR_LEVEL 0
#define DEBUG_LEVEL 1
#define INFO_LEVEL  2

#ifdef TRACE_LEVEL

#if (TRACE_LEVEL >= ERROR_LEVEL)
#define ERROR(FMT, ...) printf("%s:%d:\t%s\terror: " FMT "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define ERROR(FMT, ...)
#endif

#if (TRACE_LEVEL >= DEBUG_LEVEL)
#define DEBUG(FMT, ...) printf("%s:%d:\t%s\tdebug: " FMT "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define DEBUG(FMT, ...)
#endif

#if (TRACE_LEVEL >= INFO_LEVEL)
#define INFO(FMT, ...) printf("%s:%d:\t%s\tinfo: " FMT "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define INFO(FMT, ...)
#endif

#else

#define ERROR(FMT, ...)
#define DEBUG(FMT, ...)
#define INFO(FMT, ...)

#endif

#endif //__TRACE__H
