//Realizado por Julio Ballesteros y Álvaro Reina

#ifndef _BMP_IMAGE_LOADER_
#define _BMP_IMAGE_LOADER_

#include <stdio.h>
#include <stdlib.h>

#define uint32 unsigned int
#define uint16 unsigned short
#define uint8 unsigned char

typedef struct header_t
{
	uint8 signature[2];
	uint32 fileSize;
	uint32 reserved;
	uint32 dataOffset;
}header_t;

typedef struct infoHeader_t
{
	uint32 size;
	uint32 width;
	uint32 height;
	uint16 planes;
	uint16 bpp;
	uint32 compression;
	uint32 imageSize;
	uint32 xPixelsPerM;
	uint32 yPixelsPerM;
	uint32 colorsUsed;
	uint32 importantColors;
}infoHeader_t;

typedef struct BMPImage_t
{
	header_t header;
	infoHeader_t infoHeader;
	uint32* palette;
	uint8* pixels;
}BMPImage_t;

BMPImage_t* loadBMP(const char* path);
void writeBMP(BMPImage_t* image, const char* path);

#endif