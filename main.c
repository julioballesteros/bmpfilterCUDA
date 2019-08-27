//Realizado por Julio Ballesteros y Álvaro Reina

#include "BMPController.h"


int main(int argc, char** argv)
{
	//Cargar el archivo original
	BMPImage_t* image24 = loadBMP("FLAG_B24.bmp");
	//Crear imagenes con bpp 8 y 16 y guardarlas
	BMPImage_t* image8 = transformImage24to8(image24);
	BMPImage_t* image16 = transformImage24to16(image24);
	writeBMP(image8, "FLAG_B8.bmp");
	writeBMP(image16, "FLAG_B16.bmp");

	//Crear una nueva imagen con el filtro aplicado de cada imagen
	BMPImage_t* imgResult24 = filterImage(image24);
	BMPImage_t* imgResult8 = filterImage(image8);
	BMPImage_t* imgResult16 = filterImage(image16);

	imgResult8->header.fileSize = 14 + 40 + imgResult8->infoHeader.width*imgResult8->infoHeader.height*3;
	imgResult8->header.dataOffset = 14 + 40;
	imgResult8->infoHeader.bpp = 24;
	imgResult8->infoHeader.colorsUsed = 0;
	imgResult8->pixels = transformPixels8to24(imgResult8->pixels, imgResult8->palette, imgResult8->infoHeader.width, imgResult8->infoHeader.height);
	free(imgResult8->palette);
	imgResult8->palette = NULL;

	//Escribir las nuevas imagenes a fichero
	writeBMP(imgResult24, "imgResult24.bmp");
	writeBMP(imgResult8, "imgResult8.bmp");
	writeBMP(imgResult16, "imgResult16.bmp");

	//for (int i = 0; i < 15376; i++)
		//printf("%d\n", ((uint8*)imgResult8->pixels)[i]);
	
	//Liberar toda la memoria reservada
	free(image24->pixels);
	free(image24);
	free(image16->pixels);
	free(image16);
	free(image8->palette);
	free(image8->pixels);
	free(image8);
	free(imgResult24->pixels);
	free(imgResult24);
	free(imgResult16->pixels);
	free(imgResult16);
	free(imgResult8->pixels);
	free(imgResult8->palette);
	free(imgResult8);
	
	return 0;
}