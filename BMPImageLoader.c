//Realizado por Julio Ballesteros y Álvaro Reina

#include "BMPImageLoader.h"

header_t load_header(FILE* f_bmp)
{
	header_t header = {
		.signature = { 0, 0 },
		.fileSize = 0,
		.reserved = 0,
		.dataOffset = 0
	};

	int numBytesRead;

	numBytesRead = fread(&(header.signature[0]), 1, 2, f_bmp);
	if (numBytesRead != 2) exit(1);
	if (strncmp(&(header.signature[0]), "BM", 2) != 0) exit(1);

	numBytesRead = fread(&(header.fileSize), 1, sizeof(int), f_bmp);
	if (numBytesRead != 4) exit(1);

	numBytesRead = fread(&(header.reserved), 1, 4, f_bmp);

	numBytesRead = fread(&(header.dataOffset), 1, 4, f_bmp);
	if (numBytesRead != 4) exit(1);

	return header;
}

infoHeader_t load_infoHeader(FILE* f_bmp)
{
	infoHeader_t infoHeader = {
		.size = 0,
		.width = 0,
		.height = 0,
		.planes = 0,
		.bpp = 0,
		.compression = 0,
		.imageSize = 0,
		.xPixelsPerM = 0,
		.yPixelsPerM = 0,
		.colorsUsed = 0,
		.importantColors = 0
	};

	int numBytesRead;

	numBytesRead = fread(&(infoHeader.size), 1, 4, f_bmp);
	if (numBytesRead != 4) exit(1);

	numBytesRead = fread(&(infoHeader.width), 1, 4, f_bmp);
	if (numBytesRead != 4) exit(1);

	numBytesRead = fread(&(infoHeader.height), 1, 4, f_bmp);
	if (numBytesRead != 4) exit(1);

	numBytesRead = fread(&(infoHeader.planes), 1, 2, f_bmp);
	if (numBytesRead != 2) exit(1);

	numBytesRead = fread(&(infoHeader.bpp), 1, 2, f_bmp);
	if (numBytesRead != 2) exit(1);

	numBytesRead = fread(&(infoHeader.compression), 1, 4, f_bmp);
	if (numBytesRead != 4) exit(1);

	numBytesRead = fread(&(infoHeader.imageSize), 1, 4, f_bmp);
	if (numBytesRead != 4) exit(1);

	numBytesRead = fread(&(infoHeader.xPixelsPerM), 1, 4, f_bmp);
	if (numBytesRead != 4) exit(1);

	numBytesRead = fread(&(infoHeader.yPixelsPerM), 1, 4, f_bmp);
	if (numBytesRead != 4) exit(1);

	numBytesRead = fread(&(infoHeader.colorsUsed), 1, 4, f_bmp);
	if (numBytesRead != 4) exit(1);

	numBytesRead = fread(&(infoHeader.importantColors), 1, 4, f_bmp);
	if (numBytesRead != 4) exit(1);

	return infoHeader;
}

uint32* load_palette(FILE* file, uint16 bpp)
{
	uint32* palette = NULL;
	int numColors = 0x01 << bpp;

	int i = 0;
	//test bpp
	if (bpp > 8) return NULL;

	palette = (uint32*)malloc(sizeof(uint32)*numColors);
	for (i = 0; i < numColors; i++)
	{
		fread(&(palette[i]), 1, 4, file);
	}
	return palette;
}

uint8* load_pixels(FILE* file, uint32 bpp, uint32 width, uint32 height)
{
	uint8* pixels = NULL;
	int pixelArraySize = 0;
	int i = 0;
	switch (bpp)
	{
	case 1:
		pixelArraySize = (int)((width * height) / 8);
		break;
	case 2:
		pixelArraySize = (int)((width * height) / 4);
		break;
	case 4:
		pixelArraySize = (int)((width * height)/2);
		break;
	case 8:
		pixelArraySize = width*height;
		break;
	case 16:
		pixelArraySize = 2 * width*height;
		break;
	case 24:
		pixelArraySize = 3 * width*height;
		break;
	default:
		break;
	}

	if (pixelArraySize == 0) return pixels;

	pixels = (uint8*)malloc(pixelArraySize);
	for (i = 0; i < pixelArraySize; i++)
	{
		fread(&(pixels[i]), 1, 1, file);
	}

	return pixels;

}

BMPImage_t* loadBMP(const char* path)
{
	FILE* file = NULL;
	BMPImage_t* BMPImage = (BMPImage_t*)malloc(sizeof(BMPImage_t));
	//comprobar variables cabecera
	if (path == NULL) return NULL;
	else
	{
		//abrir fichero
		file = fopen(path, "r");
		//si no existe return
		if (file == NULL)
			return NULL;
	}

	//load header
	BMPImage->header = load_header(file);
	//load info header
	BMPImage->infoHeader = load_infoHeader(file);
	//load data
	BMPImage->palette = load_palette(file, BMPImage->infoHeader.bpp);
	BMPImage->pixels = load_pixels(file, BMPImage->infoHeader.bpp, BMPImage->infoHeader.width, BMPImage->infoHeader.height);
	fclose(file);
	//return imagen
	return BMPImage;
}

void write_header(FILE* fout, header_t header)
{
	fwrite(&(header.signature[0]), 1, 2, fout);
	fwrite(&(header.fileSize), 1, 4, fout);
	fwrite(&(header.reserved), 1, 4, fout);
	fwrite(&(header.dataOffset), 1, 4, fout);
}

void write_infoHeader(FILE* fout, infoHeader_t infoHeader)
{
	//añadir control de errores
	fwrite(&(infoHeader.size), 1, 4, fout);
	fwrite(&(infoHeader.width), 1, 4, fout);
	fwrite(&(infoHeader.height), 1, 4, fout);
	fwrite(&(infoHeader.planes), 1, 2, fout);
	fwrite(&(infoHeader.bpp), 1, 2, fout);
	fwrite(&(infoHeader.compression), 1, 4, fout);
	fwrite(&(infoHeader.imageSize), 1, 4, fout);
	fwrite(&(infoHeader.xPixelsPerM), 1, 4, fout);
	fwrite(&(infoHeader.yPixelsPerM), 1, 4, fout);
	fwrite(&(infoHeader.colorsUsed), 1, 4, fout);
	fwrite(&(infoHeader.importantColors), 1, 4, fout);
}

void write_palette(FILE* fout, infoHeader_t infoHeader, uint32* palette)
{
	int paletteArraySize = 0x01 << infoHeader.bpp;
	if (infoHeader.bpp > 8)return;
	fwrite(palette, 1, paletteArraySize * sizeof(uint32), fout);
}

void write_pixels(FILE* fout, infoHeader_t infoHeader, uint8* pixels)
{
	int pixelArraySize = 0;
	switch (infoHeader.bpp)
	{
	case 1:
		pixelArraySize = (int)((infoHeader.width * infoHeader.height) / 8);
		fwrite(pixels, 1, pixelArraySize, fout);
		break;
	case 2:
		pixelArraySize = (int)((infoHeader.width * infoHeader.height) / 4);
		fwrite(pixels, 1, pixelArraySize, fout);
		break;
	case 4:
		pixelArraySize = (int)((infoHeader.width * infoHeader.height) / 2);
		fwrite(pixels, 1, pixelArraySize, fout);
		break;
	case 8:
		pixelArraySize = infoHeader.width*infoHeader.height;
		fwrite(pixels, 1, pixelArraySize, fout);
		break;
	case 16:
		pixelArraySize = 2 * infoHeader.width*infoHeader.height;
		fwrite(pixels, 1, pixelArraySize, fout);
		break;
	case 24:
		pixelArraySize = 3 * infoHeader.width*infoHeader.height;
		fwrite(pixels, 1, pixelArraySize, fout);
		break;
	}
}

void writeBMP(BMPImage_t* image, const char* path)
{
	FILE* fout = NULL;
	//comprobar cabecera
	if ((image == NULL) || (path == NULL)) return;
	//abrir (crear) fichero
	fout = fopen(path, "w");
	//escribir cabecera
	write_header(fout, image->header);
	//escribir info cabecera
	write_infoHeader(fout, image->infoHeader);
	//escribir paleta
	write_palette(fout, image->infoHeader, image->palette);
	//escribir pixels
	write_pixels(fout, image->infoHeader, image->pixels);
	//cerrar fichero
	fclose(fout);
}