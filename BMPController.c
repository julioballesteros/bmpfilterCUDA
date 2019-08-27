//Realizado por Julio Ballesteros y Álvaro Reina

#include "BMPController.h"

BMPImage_t* filterImage(BMPImage_t* image)
{
	int i = 0;

	BMPImage_t* imgResult = (BMPImage_t*)malloc(sizeof(BMPImage_t));
	
	imgResult->header = image->header;
	imgResult->infoHeader = image->infoHeader;

	switch (imgResult->infoHeader.bpp) {
	case 8:
		imgResult->palette = (uint32*)malloc(sizeof(uint32)*256);
		memcpy(imgResult->palette, image->palette, sizeof(uint32)*256);
		imgResult->pixels = transformPixels8to24(image->pixels, image->palette, image->infoHeader.width, image->infoHeader.height);
		imgResult->pixels = applyFilterGPU(imgResult->pixels, mask, image->infoHeader.width, image->infoHeader.height, 3, 3);
		imgResult->pixels = transformPixels24to8(imgResult->pixels, imgResult->palette, image->infoHeader.width, image->infoHeader.height);
		break;

	case 16:
		imgResult->palette = NULL;
		imgResult->pixels = transformPixels16to24(image->pixels, image->infoHeader.width, image->infoHeader.height);
		imgResult->pixels = applyFilterGPU(imgResult->pixels, mask, image->infoHeader.width, image->infoHeader.height, 3, 3);
		imgResult->pixels = transformPixels24to16(imgResult->pixels, image->infoHeader.width, image->infoHeader.height);
		break;

	case 24:
		imgResult->palette = NULL;
		imgResult->pixels = applyFilterGPU(image->pixels, mask, image->infoHeader.width, image->infoHeader.height, 3, 3);
		break;

	default:
		break;
	}

	return imgResult;
}

uint8* transformPixels8to24(uint8* pixels, uint8* palette, uint32 imgW, uint32 imgH)
{
	uint32 i = 0, j = 0;
	colorPalette_t color;

	uint8* newPixels = NULL;
	
	newPixels = (uint8*)malloc(sizeof(uint8)* imgW * imgH * 3);

	memset(newPixels, 0xFF, (sizeof(uint8)* imgW * imgH * 3));

	for (i = 0; i < imgH; i++)
		for (j = 0; j < imgW; j++) {
			color = ((colorPalette_t*)palette)[pixels[i * imgW + j]];
			((pixel_t*)newPixels)[i * imgW + j].R = color.R;
			((pixel_t*)newPixels)[i * imgW + j].G = color.G;
			((pixel_t*)newPixels)[i * imgW + j].B = color.B;
		}

	return newPixels;
}

uint8* transformPixels16to24(uint8* pixels, uint32 imgW, uint32 imgH)
{
	uint32 i = 0, j = 0;
	uint16 pixOrig = NULL;
	uint16 maskR = 0x7C00;
	uint16 maskG = 0x03E0;
	uint16 maskB = 0x001F;


	uint8* newPixels = NULL;

	newPixels = (uint8*)malloc(sizeof(uint8)* imgW * imgH * 3);

	memset(newPixels, 0xFF, (sizeof(uint8)* imgW * imgH * 3));

	for (i = 0; i < imgH; i++)
		for (j = 0; j < imgW; j++) {
			pixOrig = ((uint16*)pixels)[i *imgW + j];
			((pixel_t*)newPixels)[i * imgW + j].R = (pixOrig & maskR) >> 7;
			((pixel_t*)newPixels)[i * imgW + j].G = (pixOrig & maskG) >> 2;
			((pixel_t*)newPixels)[i * imgW + j].B = (pixOrig & maskB) << 3;
		}

	return newPixels;
}

uint8* transformPixels24to16(uint8* pixels, uint32 imgW, uint32 imgH)
{
	uint32 i = 0, j = 0;
	pixel_t pixOrig;
	uint16 pixResult;
	uint8* newPixels = NULL;


	pixOrig.R = pixOrig.G = pixOrig.B = 0;

	newPixels = (uint8*)malloc(sizeof(uint8)* imgW * imgH * 2);

	memset(newPixels, 0xFF, (sizeof(uint8)* imgW * imgH * 2));

	for (i = 0; i < imgH; i++)
		for (j = 0; j < imgW; j++) {
			pixResult = 0x0000;
			pixOrig = ((pixel_t*)pixels)[i *imgW + j];
			pixResult |= (pixOrig.R >> 3) << 10;
			pixResult |= (pixOrig.G >> 3) << 5;
			pixResult |= pixOrig.B >> 3;

			((uint16*)newPixels)[i * imgW + j] = pixResult;
		}

	return newPixels;
}

uint8* transformPixels24to8(uint8* pixels, uint8* palette, uint32 imgW, uint32 imgH)
{
	bool itsUsed = false;
	uint32 numColors = 0, k, mostUsed = 0, numUsedColors = 0;
	uint8 index = 0;
	uint32 i = 0, j = 0;
	pixel_t pixOrig;
	colorPalette_t color;
	uint8 pixResult;
	uint8* newPixels = NULL;
	uint8* extraPalette = NULL;
	uint32* colorCount = NULL;
	uint32* usedColors = NULL;

	newPixels = (uint8*)malloc(sizeof(uint8)* imgW * imgH);
	extraPalette = (uint32*)malloc(sizeof(uint32)* imgW * imgH);
	colorCount = (uint32*)malloc(sizeof(uint32)* imgW * imgH);
	usedColors = (uint32*)malloc(sizeof(uint32)* 256);

	memset(newPixels, 0xFF, sizeof(uint8)* imgW * imgH);
	memset(palette, 0xFF, sizeof(uint32) * 256);
	memset(extraPalette, 0xFF, sizeof(uint32)* imgW * imgH);
	memset(colorCount, 0x00, sizeof(uint32)* imgW * imgH);
	memset(usedColors, 0x00, sizeof(uint32)* 256);
	pixOrig.R = pixOrig.G = pixOrig.B = 0;

	//Recorremos la imagen comprobando el color de cada pixel, si no lo hemos visto lo metemos en la lista de colores, si lo hemos visto aumentamos el contador
	for (i = 0; i < imgH; i++)
		for (j = 0; j < imgW; j++) {
			pixOrig = ((pixel_t*)pixels)[i *imgW + j];
			color.R = pixOrig.R;
			color.G = pixOrig.G;
			color.B = pixOrig.B;
			color.reserved = 0;

			for (k = 0; k < numColors; k++) {
				if (color.R == ((colorPalette_t*)extraPalette)[k].R && color.G == ((colorPalette_t*)extraPalette)[k].G && color.B == ((colorPalette_t*)extraPalette)[k].B) {
					colorCount[k]++;
					break;
				}
			}
			if (k == numColors) {
				((colorPalette_t*)extraPalette)[k] = color;
				colorCount[k]++;
				numColors++;

			}
		}

	for (i = 0; i < 256; i++) {
		//Buscar los 256 colores más usados dentro de la lista de colores usados y pasarlos a la paleta
		mostUsed = numColors;
		for (k = 0; k < numColors; k++) {
			itsUsed = false;
			for (j = 0; j < numUsedColors; j++) {
				if (usedColors[j] == k) itsUsed = true;
			}
			if (itsUsed) continue;
			if (colorCount[k] > colorCount[mostUsed]) {
				mostUsed = k;
			}
		}
		if (mostUsed == numColors)	break;
		((colorPalette_t*)palette)[i] = ((colorPalette_t*)extraPalette)[mostUsed];
		usedColors[numUsedColors] = mostUsed;
		numUsedColors++;
	}

	//Por cada pixel buscamos si está el color en la paleta
	for (i = 0; i < imgH; i++)
		for (j = 0; j < imgW; j++) {
			pixOrig = ((pixel_t*)pixels)[i *imgW + j];
			for (index = 0; index < 256; index++) {
				color = ((colorPalette_t*)palette)[index];
				//Si esta en la paleta
				if (color.R == pixOrig.R && color.G == pixOrig.G && color.B == pixOrig.B)	break;
			}
			//Si no esta
			if (index == 256) {
				for (k = 0; k <= 8; k++) {
					pixOrig.R = pixOrig.R >> 1;
					pixOrig.G = pixOrig.G >> 1;
					pixOrig.B = pixOrig.B >> 1;

					for (index = 0; index < 256; index++) {
						color = ((colorPalette_t*)palette)[index];
						color.R = color.R >> (k + 1);
						color.G = color.G >> (k + 1);
						color.B = color.B >> (k + 1);
						if (color.R == pixOrig.R && color.G == pixOrig.G && color.B == pixOrig.B)	break;
					}
					if (index != 256)	break;
				}
			}

			((uint8*)newPixels)[i * imgW + j] = index;
		}

	free(extraPalette);
	free(colorCount);
	free(usedColors);

	return newPixels;
}

BMPImage_t* transformImage24to16(BMPImage_t* image) {

	BMPImage_t* imgResult = (BMPImage_t*)malloc(sizeof(BMPImage_t));

	imgResult->header = image->header;
	imgResult->header.fileSize = 14 + 40 + image->infoHeader.width*image->infoHeader.height * 2;
	imgResult->infoHeader = image->infoHeader;
	imgResult->infoHeader.bpp = 16;
	imgResult->palette == NULL;
	imgResult->pixels = transformPixels24to16(image->pixels, image->infoHeader.width, image->infoHeader.height);

	return imgResult;
}

BMPImage_t* transformImage24to8(BMPImage_t* image) {

	BMPImage_t* imgResult = (BMPImage_t*)malloc(sizeof(BMPImage_t));

	imgResult->header = image->header;
	imgResult->header.fileSize = 14 + 40 + 256*4 + image->infoHeader.width*image->infoHeader.height;
	imgResult->header.dataOffset = 14 + 40 + 256*4;
	imgResult->infoHeader = image->infoHeader;
	imgResult->infoHeader.bpp = 8;
	imgResult->infoHeader.compression = 0;
	imgResult->infoHeader.colorsUsed = 256;
	imgResult->palette = (uint32*)malloc(sizeof(uint32) * 256);
	imgResult->pixels = transformPixels24to8(image->pixels, imgResult->palette, image->infoHeader.width, image->infoHeader.height);

	return imgResult;
}