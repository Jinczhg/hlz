/*
 * $QNXLicenseC:
 * Copyright 2013, QNX Software Systems.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#ifndef HDRINCL_WFDCFG_OMAP_4_5_J6
#define HDRINCL_WFDCFG_OMAP_4_5_J6

#include <wfdqnx/wfdcfg.h>

/* Common extensions */
/** Gives whether we need to invert the data line on the port. This is a port extension (HDMI port only).
 *   .p must be NULL
 *   .i set to 1 to invert data line, set to 0 to not invert the data line.
 */
#define WFDCFG_EXT_INVERT_DATA_LINE "invert_data_line"

/** Gives port the color output mode in bit per pixel. This is a port extension (LCD port only).
 *   .p must be NULL
 *  .i give the color output mode in bit per pixel
 */
#define WFDCFG_EXT_OUTPUT_MODE_IN_BPP "output_mode_in_bpp"

/** Gives the system clock in kilohertz.  This is a device extension.
 *   .p must be NULL
 *   .i give the clock speed in kilohertz.
 */
#define WFDCFG_EXT_SYSTEM_CLOCK_KHZ "system_clock_khz"

/* Extension function definition */
#define WFDCFG_EXT_FN_PORT_INIT "port_init"
typedef int (wfdcfg_ext_fn_port_init_t)(void);

#define WFDCFG_EXT_FN_PORT_UNINIT "port_uninit"
typedef int (wfdcfg_ext_fn_port_uninit_t)(void);

#define WFDCFG_EXT_FN_PORT_SET_MODE "port_set_mode"
typedef int (wfdcfg_ext_fn_port_set_mode_t)(const struct wfdcfg_timing *mode);

#endif // HDRINCL_WFDCFG_OMAP_4_5_J6


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn/product/branches/6.6.0/trunk/hardware/wfd/omap4-5-j6/wfdcfg/public/wfdqnx/wfdcfg_omap4-5-j6.h $ $Rev: 728004 $")
#endif
