//Realizado por Julio Ballesteros
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kernel.h"
#define BLOCK_SIZE 512

float mask[] = {
	0.1,0.1,0.1,
	0.1,0.1,0.1,
	0.1,0.1,0.1
};

extern __shared__ uint8 temp[];


__global__ void filtro_k(uint8* img, float* filtro, uint32 imgW, uint32 imgH, uint32 filW, uint32 filH, uint8* imgResult)
{
	
	int gi, gj, lindex, gindex, x, y, threadID;
	uint8 pixOrig;
	uint8 pixResult;

	threadID = blockIdx.x*blockDim.x + threadIdx.x;
	if (threadID >= (imgW*imgH)) return;
	
	gi = threadID / imgW;
	gj = threadID % imgW;

	gindex = gi * imgW + gj;
	lindex = threadIdx.x + imgW + 3;

	temp[lindex] = img[gindex];

	if (threadIdx.x < imgW + 3) {
		if(gindex >= imgW + 3)	temp[lindex - (imgW + 3)] = img[gindex - (imgW + 3)];
		if(gindex < imgW * imgH - (imgW + 3))	temp[lindex + blockDim.x] = img[gindex + blockDim.x];
	}

	if (gi == 0 || gi == imgH - 1)	return;
	if (gj < 3 || gj >= imgW - 3)	return;

	__syncthreads();

	pixOrig = 0;
	pixResult = 0;
	
	for (x = -1; x < 2; x++)
		for (y = -1; y < 2; y++) {
			pixOrig = temp[lindex + x*imgW + y*3];
			pixResult += pixOrig*filtro[(x + 1) + ((y + 1) * 3)];
		}
		
	imgResult[gindex] = pixResult;
}


uint8* applyFilterGPU(uint8* img, float* filtro, uint32 imgW, uint32 imgH, uint32 filW, uint32 filH)
{
	uint8 i, j;
	uint8* img_d = NULL;
	float* filtro_d = NULL;
	uint8* imgResult_d = NULL;
	uint8* imgResult_h = NULL;
	uint8* amplifiedImg = NULL;
	uint8* imgResult = NULL;

	cudaMalloc((void**)&img_d, sizeof(uint8)* 3 * (imgW + 2)*(imgH + 2));
	cudaMalloc((void**)&imgResult_d, sizeof(uint8)*(imgW + 2)*(imgH + 2) * 3);
	cudaMalloc((void**)&filtro_d, sizeof(float)*filH * filW);
	imgResult_h = (uint8*)malloc(sizeof(uint8)*(imgW + 2)*(imgH + 2) * 3);
	amplifiedImg = (uint8*)malloc(3 * (imgW + 2)*(imgH + 2));
	imgResult = (uint8*)malloc(sizeof(uint8)*imgW*imgH * 3);

	memset(amplifiedImg, 0, 3 * (imgW + 2)*(imgH + 2));
	for (i = 1; i <= imgH; i++)
		for (j = 1; j <= imgW; j++) {
			((pixel_t*)amplifiedImg)[j + i * (imgW + 2)] = ((pixel_t*)img)[(j - 1) + (i - 1)*imgW];
		}

	cudaMemcpy(img_d, amplifiedImg, sizeof(uint8) * 3 * (imgW + 2)*(imgH + 2), cudaMemcpyHostToDevice);
	cudaMemcpy(filtro_d, filtro, sizeof(float)*filW * filH, cudaMemcpyHostToDevice);

	cudaMemset(imgResult_d, 0xFF, sizeof(uint8)*(imgW + 2)*(imgH + 2) * 3);
	memset(imgResult_h, 0xFF, sizeof(uint8)* (imgW + 2)*(imgH + 2) * 3);
	memset(imgResult, 0xFF, sizeof(uint8)* imgW * imgH * 3);

	int numThreadsBloque = BLOCK_SIZE;
	int numBloques = ((imgW + 2)*(imgH + 2)*3) / numThreadsBloque + 1;

	filtro_k << < numBloques, numThreadsBloque, sizeof(uint8) * BLOCK_SIZE + ((imgW + 2 + 1) * 2) * 3 >> > (img_d, filtro_d, (imgW + 2)*3, imgH + 2, filW, filH, imgResult_d);
	cudaDeviceSynchronize();

	cudaMemcpy(imgResult_h, imgResult_d, sizeof(uint8)*(imgW + 2)*(imgH + 2) * 3, cudaMemcpyDeviceToHost);

	for (i = 0; i < imgH; i++)
		for (j = 0; j < imgW; j++) {
			((pixel_t*)imgResult)[j + i * imgW] = ((pixel_t*)imgResult_h)[(j + 1) + (i + 1)*(imgW + 2)];
		}

	cudaFree(img_d);
	cudaFree(filtro_d);
	cudaFree(imgResult_d);
	free(imgResult_h);

	return imgResult;
}

uint8* applyFilterCPU(uint8* img, float* filtro, uint32 imgW, uint32 imgH, uint32 filW, uint32 filH)
{
	uint32  i, j;
	int x, y;
	pixel_t pixOrig;
	pixel_t pixResult;
	uint8* imgResult = NULL;
	uint8* amplifiedImg = NULL;

	imgResult = (uint8*)malloc(sizeof(uint8)*imgW*imgH * 3);
	memset(imgResult, 0xFF, sizeof(uint8)*imgW*imgH * 3);

	amplifiedImg = (uint8*)malloc(3 * (imgW + 2)*(imgH + 2));
	memset(amplifiedImg, 0, 3 * (imgW + 2)*(imgH + 2));
	for (i = 1; i <= imgH; i++)
		for (j = 1; j <= imgW; j++) {
			((pixel_t*)amplifiedImg)[j + i * (imgW + 2)] = ((pixel_t*)img)[(j - 1) + (i - 1)*imgW];
		}

	for (i = 0; i < imgH; i++)
		for (j = 0; j < imgW; j ++) {

			pixOrig.R = pixOrig.G = pixOrig.B = 0;
			pixResult.R = pixResult.G = pixResult.B = 0;

			for (y = -1; y < 2; y++)
				for (x = -1; x < 2; x++)
				{
					pixOrig = ((pixel_t*)amplifiedImg)[(j + x + 1) + ((i + y + 1) * (imgW + 2))];
					pixResult.R += pixOrig.R*filtro[(x + 1) + ((y + 1) * 3)];
					pixResult.G += pixOrig.G*filtro[(x + 1) + ((y + 1) * 3)];
					pixResult.B += pixOrig.B*filtro[(x + 1) + ((y + 1) * 3)];
				}

			((pixel_t*)imgResult)[j + i * imgW] = pixResult;

		}
	return imgResult;
}
