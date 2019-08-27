//Realizado por Julio Ballesteros y Álvaro Reina

#ifndef __FILTRO_K_H__
#define __FILTRO_K_H__

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BMPImageLoader.h"

typedef struct pixel_t
{
	uint8 B;
	uint8 G;
	uint8 R;
}pixel_t;

#ifdef __cplusplus
extern "C"
{
#endif

	extern float mask[];

	uint8* applyFilterCPU(uint8* img, float* filtro, uint32 imgW, uint32 imgH, uint32 filW, uint32 filH);
	uint8* applyFilterGPU(uint8* img, float* filtro, uint32 imgW, uint32 imgH, uint32 filW, uint32 filH);

#ifdef __cplusplus
}
#endif
#endif
