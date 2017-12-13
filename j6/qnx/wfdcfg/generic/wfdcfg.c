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

//#include <wfdqnx/wfdcfg.h>
#include <wfdqnx/wfdcfg_omap4-5-j6.h>

#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>  // NULL
#include <string.h>

#define MODELINE(mhz, h1, h2, h3, h4, v1, v2, v3, v4, f) \
	{ .pixel_clock_kHz = (mhz) * 1000, \
	.hpixels = (h1), .hfp = (h2) - (h1), .hsw = (h3) - (h2), .hbp = (h4) - (h3), \
	.vlines  = (v1), .vfp = (v2) - (v1), .vsw = (v3) - (v2), .vbp = (v4) - (v3), \
	.flags = (f), } \


struct wfdcfg_device {
	const struct wfdcfg_keyval *ext_list;
};

struct wfdcfg_port {
	int id;
	const struct wfdcfg_keyval *ext_list;
};

struct wfdcfg_mode_list {
	const struct wfdcfg_timing_internal *first_mode;
};

/* Internal structure to keep a mode and
 * its associated extension(s).
 * */
struct wfdcfg_timing_internal {
	const struct wfdcfg_timing mode;
	const struct wfdcfg_keyval *mode_ext;
};

/* Helper function(s) */
static const struct wfdcfg_timing_internal*
cast_timing_to_timing_ext(const struct wfdcfg_timing *timing) {
	char *p = (char*)timing - offsetof(struct wfdcfg_timing_internal, mode);
	return (const struct wfdcfg_timing_internal*)p;
}

static const struct wfdcfg_keyval* get_ext_from_list(const struct wfdcfg_keyval *ext_list, const char *key) {
	while (ext_list) {
		if (!ext_list->key) {
			ext_list = NULL;
			break;
		} else if (strcmp(ext_list->key, key) == 0) {
			return ext_list;
		}
		++ext_list;
	}

    return NULL;
}


static const struct wfdcfg_keyval lcd_port_ext_rgb666[] = {
	{
		.key = WFDCFG_EXT_OUTPUT_MODE_IN_BPP,
		.i = 18,
		.p = NULL,
	},
	{ NULL }  // marks end of list
};

static const struct wfdcfg_keyval lcd_port_ext_rgb888[] = {
	{
		.key = WFDCFG_EXT_OUTPUT_MODE_IN_BPP,
		.i = 24,
		.p = NULL,
	},
	{ NULL }  // marks end of list
};

static const struct wfdcfg_timing_internal sample_timings[] = {
	{
		 // 1280*800 @ 60 Hz for HSD101PWW1
		.mode =  {
			.pixel_clock_kHz = 71000,
			.hpixels = 1280, .hfp = 80, .hsw = 32, .hbp = 48, // 1440 total
			.vlines  = 800,  .vfp = 14,  .vsw = 6,  .vbp = 3, //  823 total
			//.flags   = NULL,
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = NULL,
       },
		 /* 1920x1200 (0xb4)  154.0MHz +HSync -VSync +preferred for HDMI
                h: width  1920 start 1968 end 2000 total 2080 skew    0 clock   74.0KHz
                v: height 1200 start 1203 end 1209 total 1235           clock   60.0Hz */             
       {
		.mode = MODELINE(154.0, 1920, 1968, 2000, 2080, 1200, 1203, 1209, 1235, WFDCFG_INVERT_VSYNC),
		.mode_ext = NULL,
       },
       {
		 // 1920*1200 @ 50 Hz 128.4MHz, 17' LCD  
		.mode =  {
			.pixel_clock_kHz = 128440,
			.hpixels = 1920, .hfp = 48, .hsw = 32, .hbp = 80, // 2080 total
			.vlines  = 1200,  .vfp = 3,  .vsw = 6,  .vbp = 26, //  1235 total
			//.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
			.flags   = WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = NULL,
       },
       {
		 // 1920*1200 @ 45 Hz 115.6MHz, 17' LCD  
		.mode =  {
			.pixel_clock_kHz = 115596,
			.hpixels = 1920, .hfp = 48, .hsw = 32, .hbp = 80, // 2080 total
			.vlines  = 1200,  .vfp = 3,  .vsw = 6,  .vbp = 26, //  1235 total
			//.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
			.flags   = WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = NULL,
       },
       {
		 // 1920*1200 @ 30 Hz 77MHz, 17' LCD  
		.mode =  {
			.pixel_clock_kHz = 77064,
			.hpixels = 1920, .hfp = 48, .hsw = 32, .hbp = 80, // 2080 total
			.vlines  = 1200,  .vfp = 3,  .vsw = 6,  .vbp = 26, //  1235 total
			//.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
			.flags   = WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = NULL,
       },
#if 0 //only for revB
	{
		 // 1366*768 @ 59 Hz for RGB->LVDS CHANGAN
		 // (74180*1000) / (1560*806) = 58.99 Hz
		.mode =  {
			.pixel_clock_kHz = 74180,
			.hpixels = 1366, .hfp = 190, .hsw = 2, .hbp = 2,   // 1366+194=1560 total
			.vlines  = 768,  .vfp = 1,   .vsw = 1, .vbp = 36,  // 768+38=806 total
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = NULL,
	},
#endif
	{
		// 720p @ 60 Hz
		.mode =  {
			.pixel_clock_kHz = 74250,
			.hpixels = 1280, .hfp = 110, .hsw = 40, .hbp = 220, // 1650 total
			.vlines  = 720,  .vfp = 5,   .vsw = 5,  .vbp = 20,  // 750 total
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = (const struct wfdcfg_keyval[]){
			{ "ceamode_code", .i = 4, .p = NULL }, // See CEA 861-D document at page 12 for code.
			{ NULL }  // marks end of list
		},
	},
	{
		 // 1080p @ 60 Hz
		.mode =  {
			.pixel_clock_kHz = 148500,
			.hpixels = 1920, .hfp = 88, .hsw = 44, .hbp = 148,   // 2200 total
			.vlines  = 1080, .vfp = 4,  .vsw = 5,  .vbp = 36,    // 1125 total
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = (const struct wfdcfg_keyval[]){
			{ "ceamode_code", .i = 16, .p = NULL }, // See CEA 861-D document at page 12 for code.
			{ NULL }  // marks end of list
		},
	},
	{
		// 1080i @ 60 Hz
		.mode =  {
			.pixel_clock_kHz = 74250,
			.hpixels = 1920, .hfp = 528, .hsw = 44, .hbp = 148,
			.vlines  = 1080, .vfp = 2,  .vsw = 5,  .vbp = 15,
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INTERLACE | WFDCFG_INVERT_CLOCK ,
		},
		.mode_ext = NULL,
	},
	{
		// 800x480 @ 60 Hz
		.mode =  {
			.pixel_clock_kHz = 32665,
			.hpixels = 800, .hfp = 28, .hsw = 181, .hbp = 28, // 1037 total
			.vlines  = 480, .vfp = 5,  .vsw = 20,  .vbp = 20, //  525 total
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = NULL,
	},
	{
		// 800x600 @ 60.3 Hz
		.mode =  {
			.pixel_clock_kHz = 40000,
			.hpixels = 800, .hfp = 40, .hsw = 128, .hbp = 88, // 1056 total
			.vlines  = 600, .vfp = 1,  .vsw = 4,  .vbp = 23, //  628 total
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = NULL,
	},
	{
		// 1024x600 @ 60 Hz (CVT)
		// (49000*1000) / (1312*624) = 59.85 Hz
		.mode = {
			.pixel_clock_kHz =  49000,
			.hpixels = 1024, .hfp= 40, .hsw=104, .hbp=144,  // 1312 total
			.vlines  =  600, .vfp=  3, .vsw= 10, .vbp= 11,  //  624 total
			.flags = WFDCFG_INVERT_VSYNC,
		},
		.mode_ext = NULL,
	},
	{
		// 1024*768 @ 60 Hz
		.mode =  {
			.pixel_clock_kHz = 65000,
			.hpixels = 1024, .hfp = 24, .hsw = 136, .hbp = 160, // 1344 total
			.vlines  = 768,  .vfp = 3,  .vsw = 6,   .vbp = 29,  //  806 total
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK ,
		},
		.mode_ext = NULL,
	},
	{
		// 1280*768 @ 60 Hz
		.mode =  {
			.pixel_clock_kHz = 79500,
			.hpixels = 1280, .hfp = 64, .hsw = 128, .hbp = 192, // 1664 total
			.vlines  = 768,  .vfp = 3,  .vsw = 7,   .vbp = 20,  // 798 total
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = NULL,
	},
	{
		 // 1280*1024 @ 60 Hz
		.mode =  {
			.pixel_clock_kHz = 108000,
			.hpixels = 1280, .hfp = 48, .hsw = 112, .hbp = 248, // 1688 total
			.vlines  = 1024, .vfp = 1,  .vsw = 3,   .vbp = 38,  // 1066 total
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = NULL,
	},
	{
		 // 1366*768 @ 60 Hz
		.mode =  {
			.pixel_clock_kHz = 85500,
			.hpixels = 1366, .hfp =70, .hsw = 143, .hbp = 213, // 1792 total
			.vlines  = 768,  .vfp = 3, .vsw = 3,   .vbp = 24,  // 798 total
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = NULL,
	},
	{
		// 1280*800 @ 70 Hz
		.mode =  {
			.pixel_clock_kHz = 82128,
			.hpixels = 1280, .hfp = 48, .hsw = 32, .hbp = 80, // 1440 total
			.vlines  = 800,  .vfp = 2,  .vsw = 6,  .vbp = 15, //  823 total
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = NULL,
	},
	{
		// 1280*800 @ 57 Hz Reduced blanking VESA CVT 0.31M3-R
		.mode = {
			.pixel_clock_kHz = 67333,
			.hpixels = 1280, .hfp = 32, .hsw = 48, .hbp = 80, // 1440 total
			.vlines = 800, .vfp = 4, .vsw = 3, .vbp = 7, // 814 total
			.flags = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = NULL,
	},
	/********************************************************************************/
	/*************************** Chang An Demo **************************************/
	/********************************************************************************/
#if 1 //only for revB
	{
		 // 720*576 @ 50 Hz for HDMI->CVBS
		 // (27000*1000) / (864*625) = 50 Hz
		.mode =  {
			.pixel_clock_kHz = 27000,
			.hpixels = 720, .hfp = 12, .hsw = 64, .hbp = 68,  // 864 total
			.vlines  = 576, .vfp = 5,  .vsw = 5,  .vbp = 39,  // 625 total
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = (const struct wfdcfg_keyval[]){
			{ "ceamode_code", .i = 17, .p = NULL }, // See CEA 861-D document at page 12 for code.
			{ "ceamode_code", .i = 18, .p = NULL },
			{ NULL }  // marks end of list
		},
	},
	/* 1920x720 @ 60Hz 97.34MHz +HSync +VSync
		h: width  1920 start 1980 end 2000 total 2080 skew    0
		v: height  720 start  722 end  723 total  780           */
 	{
		.mode = MODELINE(97.34, 1920, 1980, 2000, 2080, 720, 722, 723, 780, 0),
		.mode_ext = NULL,
	},
		/* 1920x720 @ 28Hz 45.43MHz +HSync +VSync
		h: width  1920 start 1980 end 2000 total 2080 skew    0
		v: height  720 start  722 end  723 total  780           */
 	{
		.mode = MODELINE(45.43, 1920, 1980, 2000, 2080, 720, 722, 723, 780, 0),
		.mode_ext = NULL,
	},
	{
		// 1440*576 @ 50 Hz
		.mode =  {
			.pixel_clock_kHz = 54000,
			.hpixels = 1440, .hfp = 24, .hsw = 128, .hbp = 136,  // 1728 total
			.vlines  = 576, .vfp = 5,  .vsw = 5,  .vbp = 39,  //625 total
			.flags   = WFDCFG_INVERT_CLOCK,
			},
#if 1
		.mode_ext = NULL,
#else
		.mode_ext = (const struct wfdcfg_keyval[]){
			{ "ceamode_code", .i = 29, .p = NULL }, // See CEA 861-D document at page 12 for code.
			{ "ceamode_code", .i = 30, .p = NULL },
			{ NULL }  // marks end of list
		},
#endif
	},
	{
		// 1440*576i @ 50 Hz
		.mode =  {
			.pixel_clock_kHz = 27000,
			.hpixels = 720, .hfp = 24, .hsw = 846, .hbp = 138,  // 1728 total
			.vlines  = 576, .vfp = 2,  .vsw = 28,  .vbp = 19,  //625 total
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INTERLACE | WFDCFG_INVERT_CLOCK,
			},
		//.mode_ext = NULL,
		.mode_ext = (const struct wfdcfg_keyval[]){
			{ "ceamode_code", .i = 21, .p = NULL }, // See CEA 861-D document at page 12 for code.
			{ "ceamode_code", .i = 22, .p = NULL },
			{ NULL }  // marks end of list
		},
	},
#endif
	{
		 // 958*720 @ 30 Hz for FPD(925)->FPD(928)
		  // (28723*1000) / (1088*880) = 30 Hz
		.mode =  {
			.pixel_clock_kHz = 28723,
			.hpixels = 958, .hfp = 30, .hsw =30 , .hbp = 70,		//1088
			.vlines  = 720,  .vfp = 40, .vsw = 40,   .vbp = 80,		//880
			.flags   = WFDCFG_INVERT_HSYNC | WFDCFG_INVERT_VSYNC | WFDCFG_INVERT_CLOCK,
		},
		.mode_ext = NULL,
	},
	{
		// marks end of list
		.mode = {.pixel_clock_kHz = 0},
	},
};

int
wfdcfg_device_create(struct wfdcfg_device **device, int deviceid,
	const struct wfdcfg_keyval *opts) {
	int err = EOK;
	struct wfdcfg_device *tmp_dev = NULL;
	(void)opts;

	switch(deviceid) {
		case 1:
			tmp_dev = malloc(sizeof(*tmp_dev));
			if(!tmp_dev) {
				err = ENOMEM;
				goto end;
			}

			tmp_dev->ext_list = NULL;
			break;
		default:
			/* Invalid device id*/
			err = ENOENT;
			goto end;
	}

end:
	if(err) {
		free(tmp_dev);
	} else {
		*device = tmp_dev;
	}

	return err;

}

const struct wfdcfg_keyval*
wfdcfg_device_get_extension(const struct wfdcfg_device *device, const char *key) {
	return get_ext_from_list(device->ext_list, key);
}

void
wfdcfg_device_destroy(struct wfdcfg_device *device) {
	free(device);
}

int
wfdcfg_port_create(struct wfdcfg_port **port, const struct wfdcfg_device *device, int portid,
	const struct wfdcfg_keyval *opts) {
	int err = EOK;
	struct wfdcfg_port *tmp_port = NULL;
	(void)opts;

	assert(device);

	switch(portid) {
		case 1:
		case 3:
		case 4:
#if 0
			tmp_port = malloc(sizeof(*tmp_port));
			if(!tmp_port) {
				err = ENOMEM;
				goto end;
			}
			tmp_port->id = portid;
			break;
#endif
		case 2:
			tmp_port = malloc(sizeof(*tmp_port));
			if(!tmp_port) {
				err = ENOMEM;
				goto end;
			}
			tmp_port->id = portid;
			//tmp_port->ext_list = NULL;
			/* Select the lcd color output mode */
			tmp_port->ext_list = lcd_port_ext_rgb888;			
			break;
		default:
			/* Invalid port id*/
			err = ENOENT;
			goto end;
	}

end:
	if(err) {
		free(tmp_port);
	} else {
		*port = tmp_port;
	}

	return err;
}

const struct wfdcfg_keyval*
wfdcfg_port_get_extension(const struct wfdcfg_port *port, const char *key) {
	return get_ext_from_list(port->ext_list, key);
}

void
wfdcfg_port_destroy(struct wfdcfg_port *port) {
	free(port);
}

int
wfdcfg_mode_list_create(struct wfdcfg_mode_list **list,
	const struct wfdcfg_port* port, const struct wfdcfg_keyval *opts) {

	int err = 0;
	const struct wfdcfg_timing_internal *first_mode;
	struct wfdcfg_mode_list *tmp_mode_list = NULL;

	(void)opts;

	assert(port);

	switch (port->id) {
	case 1:
	case 2:
	case 3:
	case 4:
		first_mode = &sample_timings[0];
		break;
	default:
		err = ENOENT;
		goto out;
	}

	tmp_mode_list = malloc(sizeof *tmp_mode_list);
	if (!tmp_mode_list) {
		err = ENOMEM;
		goto out;
	}
	tmp_mode_list->first_mode = first_mode;
out:
	if (err) {
		free(tmp_mode_list);
	} else {
		*list = tmp_mode_list;
	}
	return err;
}

const struct wfdcfg_keyval*
wfdcfg_mode_list_get_extension(const struct wfdcfg_mode_list *mode_list, const char *key) {
	(void)mode_list;
	(void)key;
	return NULL;
}

void
wfdcfg_mode_list_destroy(struct wfdcfg_mode_list *list) {
	free(list);
}

const struct wfdcfg_timing*
wfdcfg_mode_list_get_next(const struct wfdcfg_mode_list *list,
	const struct wfdcfg_timing *prev_mode) {

	assert(list);

	const struct wfdcfg_timing_internal *m = list->first_mode;
	if (prev_mode) {
		m = cast_timing_to_timing_ext(prev_mode) + 1;
	}

	if (m->mode.pixel_clock_kHz == 0) {
		// end of list (this is not an error)
		m = NULL;
	}
	return &m->mode;
}

const struct wfdcfg_keyval*
wfdcfg_mode_get_extension(const struct wfdcfg_timing *mode,
	const char *key) {

	const struct wfdcfg_keyval *ext = cast_timing_to_timing_ext(mode)->mode_ext;
	return get_ext_from_list(ext, key);
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/wfd/omap4-5-j6/wfdcfg/generic/wfdcfg.c $ $Rev: 746290 $")
#endif
