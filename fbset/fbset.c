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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <ctype.h>
#include <sys/stat.h>
#include <linux/fb.h>

#define FBPATH "/dev/graphics/fb0"

void usage() {
	printf ("usage: fbset X Y : i.e. fbset 720 1184");
	exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
	int fd; 	
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;

	if(argc < 2)
		usage();


	fd = open(FBPATH, O_RDWR);

	if(fd<0) { 
		perror("Could not open framebuffer");
		return 1;
	}

	if (argc < 3) 
		usage();

	if(ioctl(fd, FBIOGET_VSCREENINFO, &var)) {
		perror("Could not get variable screen info");
		return 1;
	}

	if(ioctl(fd, FBIOGET_FSCREENINFO, &fix)) {
		perror("Could not get fixed screen info");
		return 1;
	}

	printf("fix: %16s (%d), type=%d/%d, visual=%d, pansteps=%d,%d\n", fix.id, fix.smem_len, fix.type, fix.type_aux, fix.visual, fix.xpanstep, fix.ypanstep);
	printf("var: %dx%d visible, %dx%d virtual, %dx%d mm, %d,%d offset, %d bits/pixel, %d grayscale, %x vmode\n", var.xres, var.yres, var.xres_virtual, var.yres_virtual, var.width, var.height, var.xoffset, var.yoffset, var.bits_per_pixel, var.grayscale, var.vmode);
	printf("colors: red: %d<<%d, green: %d<<%d, blue: %d<<%d, alpha; %d<<%d\n", var.red.length, var.red.offset, var.green.length, var.green.offset, var.blue.length, var.blue.offset, var.transp.length, var.transp.offset);

	// original setting
	// 1280 x 768
	var.xres = atoi(argv[1]);
	var.yres = atoi(argv[2]);
	// new setting 360X480
	//var.xres = 480;
	//var.yres = 800;

	var.xres_virtual = var.xres; 
	// needed for page flipping
	var.yres_virtual = var.yres;
//
// 1/4/2012 -- use defaults in gralloc/framebuffer.cpp
//
/*
 *
 *        // must specify real width and height for DPI-detection purposes
 *        var.width = 52;	// 52mm, roughly the same as the Galaxy S screen
 *        var.height = var.width * var.yres / var.xres;
 *        
 *
 */

	var.yoffset = var.yres;

	var.bits_per_pixel=16;
	var.grayscale=0;
	// only handle 16 bits / per pixel at the moment
	// http://www.linux-fbdev.org/HOWTO/4.html
	
	switch (var.bits_per_pixel) {
		case 1:
		case 8:
			/* Pseudocolor mode example */
			var.red.offset    = 0;
			var.red.length    = 8;
			var.green.offset  = 0;
			var.green.length  = 8;
			var.blue.offset   = 0;
			var.blue.length   = 8;
			var.transp.offset = 0;
			var.transp.length = 0;
			break;
		case 16:        /* RGB 565 */
			var.red.offset    = 0;
			var.red.length    = 5;
			var.green.offset  = 5;
			var.green.length  = 6;
			var.blue.offset   = 11;
			var.blue.length   = 5;
			var.transp.offset = 0;
			var.transp.length = 0;
			break;
		case 24:        /* RGB 888 */
			var.red.offset    = 0;
			var.red.length    = 8;
			var.green.offset  = 8;
			var.green.length  = 8;
			var.blue.offset   = 16;
			var.blue.length   = 8;
			var.transp.offset = 0;
			var.transp.length = 0;
			break;
		case 32:        /* RGBA 8888 */
			var.red.offset    = 0;
			var.red.length    = 8;
			var.green.offset  = 8;
			var.green.length  = 8;
			var.blue.offset   = 16;
			var.blue.length   = 8;
			var.transp.offset = 24;
			var.transp.length = 8;
			break;
	}

		var.vmode = FB_VMODE_YWRAP;

	if(ioctl(fd, FBIOPUT_VSCREENINFO, &var)) {
		perror("Could not set variable screen info");
		return 1;
	}

	if(ioctl(fd, FBIOGET_VSCREENINFO, &var)) {
		perror("Could not reget variable screen info");
		return 1;
	}
	printf("var: %dx%d visible, %dx%d virtual, %dx%d mm, %d,%d offset, %d bits/pixel, %d grayscale, %x vmode\n", var.xres, var.yres, var.xres_virtual, var.yres_virtual, var.width, var.height, var.xoffset, var.yoffset, var.bits_per_pixel, var.grayscale, var.vmode);
	printf("colors: red: %d<<%d, green: %d<<%d, blue: %d<<%d, alpha; %d<<%d\n", var.red.length, var.red.offset, var.green.length, var.green.offset, var.blue.length, var.blue.offset, var.transp.length, var.transp.offset);

	return 0;
}
