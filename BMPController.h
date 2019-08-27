//Realizado por Julio Ballesteros y Álvaro Reina

#ifndef _BMP_CONTROLLER_
#define _BMP_CONTROLLER_

#include "BMPImageLoader.h"
#include "kernel.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct colorPalette_t
{
	uint8 B;
	uint8 G;
	uint8 R;
	uint8 reserved;
}colorPalette_t;


BMPImage_t* filterImage(BMPImage_t* image);
uint8* transformPixels8to24(uint8* pixels, uint8* palette, uint32 imgW, uint32 imgH);
uint8* transformPixels16to24(uint8* pixels, uint32 imgW, uint32 imgH);
uint8* transformPixels24to16(uint8* pixels, uint32 imgW, uint32 imgH);
uint8* transformPixels24to8(uint8* pixels, uint8* palette, uint32 imgW, uint32 imgH);
BMPImage_t* transformImage24to16(BMPImage_t* image);
BMPImage_t* transformImage24to8(BMPImage_t* image);


#endif