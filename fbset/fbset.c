/*
Copyright 2013 The MITRE Corporation, All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this work except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


// Framebuffer Noodling

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <ctype.h>
#include <sys/stat.h>
#include <linux/fb.h>

#define FBPATH "/dev/graphics/fb0"

#include <cutils/log.h>

void usage() {
	printf ("usage: fbset X-res Y-res fps: i.e. fbset 720 1184 30");
	exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
	int fd; 	
	struct fb_var_screeninfo info;
	struct fb_fix_screeninfo fix;
	int targetFPS;
        uint64_t refreshDivisor;

	if (argc < 3) 
		usage();

	fd = open(FBPATH, O_RDWR);

	if(fd<0) { 
		perror("Could not open framebuffer");
		return 1;
	}

	if(ioctl(fd, FBIOGET_VSCREENINFO, &info)) {
		perror("Could not get infoiable screen info");
		return 1;
	}

	if(ioctl(fd, FBIOGET_FSCREENINFO, &fix)) {
		perror("Could not get fixed screen info");
		return 1;
	}

	printf("fix: %16s (%d), type=%d/%d, visual=%d, pansteps=%d,%d\n",
		fix.id, fix.smem_len, fix.type,
		fix.type_aux, fix.visual,
		fix.xpanstep, fix.ypanstep);
	printf("info: %dx%d visible, %dx%d virtual, %dx%d mm, %d,%d "
		"offset, %d bits/pixel, %d grayscale, %x vmode\n",
		info.xres, info.yres,
		info.xres_virtual, info.yres_virtual,
		info.width, info.height,
		info.xoffset, info.yoffset,
		info.bits_per_pixel, info.grayscale, info.vmode);
	printf("colors: red: %d<<%d, green: %d<<%d, blue: %d<<%d, alpha; %d<<%d\n",
		info.red.length, info.red.offset,
		info.green.length, info.green.offset,
		info.blue.length, info.blue.offset,
		info.transp.length, info.transp.offset);

        // Set the frame resolution
	info.xres = atoi(argv[1]);
	info.yres = atoi(argv[2]);

	info.xres_virtual = info.xres; 
	// allow for triple buffering page flipping
	info.yres_virtual = 3 * info.yres;
//
// 1/4/2012 -- use defaults in gralloc/framebuffer.cpp
//
/*
 *
 *	// must specify real width and height for DPI-detection purposes
 *	info.width = 52;	// 52mm, roughly the same as the Galaxy S screen
 *	info.height = info.width * info.yres / info.xres;
 *	
 *
 */

	info.yoffset = info.yres;

	/* Set the refresh timings.
	 * ============================================================
	 *
	 * According to the kernel's Documentation/fb/framebuffer.txt:
	 *   The frame buffer device uses the following fields:
	 *
	 *    - pixclock: pixel clock in ps (pico seconds)
	 *    - left_margin: time from sync to picture
	 *    - right_margin: time from picture to sync
	 *    - upper_margin: time from sync to picture
	 *    - lower_margin: time from picture to sync
	 *    - hsync_len: length of horizontal sync
	 *    - vsync_len: length of vertical sync
	 *
	 */

	// gralloc/framebuffer.cpp calculates the fps as:
	//
	// uint64_t  refreshQuotient =
	// (
	//	 uint64_t( info.upper_margin + info.lower_margin + info.yres )
	//	 * ( info.left_margin  + info.right_margin + info.xres )
	//	 * info.pixclock
	// );
	// 
	// /* Beware, info.pixclock might be 0 under emulation, so avoid a
	//  * division-by-0 here (SIGFPE on ARM) */
	// int refreshRate = refreshQuotient > 0 ? (int)(1000000000000000LLU / refreshQuotient) : 0;
	//
	// float fps  = refreshRate / 1000.0f;

	targetFPS = atoi(argv[3]);
        refreshDivisor =
	(
		(uint64_t)( info.upper_margin + info.lower_margin + info.yres )
			* ( info.left_margin + info.right_margin + info.xres )
			* targetFPS
	);
	info.pixclock = (uint32_t)( 1000000000000LLU / refreshDivisor );

	/* ========================================================== */

	info.bits_per_pixel=16;
	info.grayscale=0;
	// only handle 16 bits / per pixel at the moment
	// http://www.linux-fbdev.org/HOWTO/4.html
	
	switch (info.bits_per_pixel) {
		case 1:
		case 8:
			/* Pseudocolor mode example */
			info.red.offset    = 0;
			info.red.length    = 8;
			info.green.offset  = 0;
			info.green.length  = 8;
			info.blue.offset   = 0;
			info.blue.length   = 8;
			info.transp.offset = 0;
			info.transp.length = 0;
			break;
		case 16:	/* RGB 565 */
			info.red.offset    = 0;
			info.red.length    = 5;
			info.green.offset  = 5;
			info.green.length  = 6;
			info.blue.offset   = 11;
			info.blue.length   = 5;
			info.transp.offset = 0;
			info.transp.length = 0;
			break;
		case 24:	/* RGB 888 */
			info.red.offset    = 0;
			info.red.length    = 8;
			info.green.offset  = 8;
			info.green.length  = 8;
			info.blue.offset   = 16;
			info.blue.length   = 8;
			info.transp.offset = 0;
			info.transp.length = 0;
			break;
		case 32:	/* RGBA 8888 */
			info.red.offset    = 0;
			info.red.length    = 8;
			info.green.offset  = 8;
			info.green.length  = 8;
			info.blue.offset   = 16;
			info.blue.length   = 8;
			info.transp.offset = 24;
			info.transp.length = 8;
			break;
	}

	info.vmode = FB_VMODE_YWRAP;

	// Set the new screeninfo 
	if(ioctl(fd, FBIOPUT_VSCREENINFO, &info)) {
		perror("Could not set screen info");
		return 1;
	}

	// Get the screeninfo again to double check the values
	if(ioctl(fd, FBIOGET_VSCREENINFO, &info)) {
		perror("Could not reget screen info");
		return 1;
	}
	printf("info: %dx%d visible, %dx%d virtual, %dx%d mm, %d,%d offset, %d bits/pixel, %d grayscale, %x vmode\n", 
		info.xres, info.yres,
		info.xres_virtual, info.yres_virtual,
		info.width, info.height,
		info.xoffset, info.yoffset,
		info.bits_per_pixel,
		info.grayscale, info.vmode);
	printf("colors: red: %d<<%d, green: %d<<%d, blue: %d<<%d, alpha; %d<<%d\n",
		info.red.length, info.red.offset,
		info.green.length, info.green.offset,
		info.blue.length, info.blue.offset,
		info.transp.length, info.transp.offset);

	return 0;
}
