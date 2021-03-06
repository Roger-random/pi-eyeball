// Converter for spooky eye bitmaps
//
// Based on LibPNG example code by A.Greensted http://www.labbookpages.co.uk
//
// Because microcontrollers have limited ability to run modern image
// decompression code, the Arduino spooky eye project
//   https://learn.adafruit.com/animated-electronic-eyes-using-teensy-3-1
// converted bitmap information into a raw uncompressed byte array for
// consumption by a low power chip.
//
// For running similar code on a device with more powerful processor, say
// a Raspberry Pi, we would prefer to work with bitmap. This code consumes
// the header file from Arduino sketch and convert it into a PNG bitmap.

#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <png.h>
#include <stdint.h>

#include "defaultEye.h" // Replace with appropriate header for different eye

// This function actually writes out the PNG image file. The string 'title' is
// also written into the image file
int writeImage(char* filename, int width, int height, png_bytep* buffer, char* title);

// Allocate a buffer of width * height pixels, each pixel consists of 3 png_bytep
png_bytep* allocPNGBuffer(int width, int height)
{
	png_bytep* row_pointers = (png_bytep*)malloc(height*sizeof(png_bytep));
	int row, column;
	for (row=0; row < height; row++)
	{
		row_pointers[row] = (png_bytep)malloc(3*width*sizeof(png_bytep)); // Each pixel has three bytes for RGB
	}

	return row_pointers;
}

// Free array allocated by the above
void freePNGBuffer(int height, png_bytep* buffer)
{
	for (int row=0; row < height; row++)
	{
		free(buffer[row]);
	}
	free(buffer);
}

// Use sclera array in header file to write out sclera.png
void writeSclera()
{
	png_bytep* row_pointers = allocPNGBuffer(SCLERA_WIDTH, SCLERA_HEIGHT);

	printf("Reading sclera array\n");
	int row, column;
	for (row=0; row<SCLERA_HEIGHT; row++)
	{
		for(column=0; column<SCLERA_WIDTH; column++)
		{
			uint16_t rgb565 = sclera[row][column];

			// Shift bits to turn 16-bit RGB565 value into 24-bit RGB888
			row_pointers[row][column*3  ] = (rgb565 >> 8) & 0xF8;
			row_pointers[row][column*3+1] = (rgb565 >> 3) & 0xFC;
			row_pointers[row][column*3+2] = (rgb565 << 3) & 0xF8;
		}
	}

	printf("Writing sclera.png\n");
	writeImage("sclera.png", SCLERA_WIDTH, SCLERA_HEIGHT, row_pointers, "Sclera");

	freePNGBuffer(SCLERA_HEIGHT, row_pointers);
}

// Repeat for iris
void writeIris()
{
	png_bytep* row_pointers = allocPNGBuffer(IRIS_MAP_WIDTH, IRIS_MAP_HEIGHT);

	printf("Reading iris array\n");
	int row, column;
	for (row=0; row<IRIS_MAP_HEIGHT; row++)
	{
		for(column=0; column<IRIS_MAP_WIDTH; column++)
		{
			uint16_t rgb565 = iris[row][column];

			row_pointers[row][column*3  ] = (rgb565 >> 8) & 0xF8;
			row_pointers[row][column*3+1] = (rgb565 >> 3) & 0xFC;
			row_pointers[row][column*3+2] = (rgb565 << 3) & 0xF8;
		}
	}

	printf("Writing iris.png\n");
	writeImage("iris.png", IRIS_MAP_WIDTH, IRIS_MAP_HEIGHT, row_pointers, "Iris");

	freePNGBuffer(IRIS_MAP_HEIGHT, row_pointers);
}

// Write PNG representation of grayscale upper eyelid
void writeUpper()
{
	png_bytep* row_pointers = allocPNGBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);

	printf("Reading upper eyelid array\n");
	int row, column;
	for (row=0; row<SCREEN_HEIGHT; row++)
	{
		for(column=0; column<SCREEN_WIDTH; column++)
		{
			uint8_t grayscale = upper[row][column];

			row_pointers[row][column*3  ] = grayscale;
			row_pointers[row][column*3+1] = grayscale;
			row_pointers[row][column*3+2] = grayscale;
		}
	}

	printf("Writing upper.png\n");
	writeImage("upper.png", SCREEN_WIDTH, SCREEN_HEIGHT, row_pointers, "Upper eyelid");

	freePNGBuffer(SCREEN_HEIGHT, row_pointers);
}

// Repeat for lower eyelid
void writeLower()
{
	png_bytep* row_pointers = allocPNGBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);

	printf("Reading lower eyelid array\n");
	int row, column;
	for (row=0; row<SCREEN_HEIGHT; row++)
	{
		for(column=0; column<SCREEN_WIDTH; column++)
		{
			uint8_t grayscale = lower[row][column];

			row_pointers[row][column*3  ] = grayscale;
			row_pointers[row][column*3+1] = grayscale;
			row_pointers[row][column*3+2] = grayscale;
		}
	}

	printf("Writing lower.png\n");
	writeImage("lower.png", SCREEN_WIDTH, SCREEN_HEIGHT, row_pointers, "Lower eyelid");

	freePNGBuffer(SCREEN_HEIGHT, row_pointers);
}

// Main function writes out four spooky eye arrays into their individual PNG files
int main(int argc, char *argv[])
{
	writeSclera();
	writeIris();
	writeUpper();
	writeLower();
}

// Mostly untouched from original libPNG example
int writeImage(char* filename, int width, int height, png_bytep* buffer, char* title)
{
	int code = 0;
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	
	// Open file for writing (binary mode)
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		fprintf(stderr, "Could not open file %s for writing\n", filename);
		code = 1;
		goto finalise;
	}

	// Initialize write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "Could not allocate write struct\n");
		code = 1;
		goto finalise;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "Could not allocate info struct\n");
		code = 1;
		goto finalise;
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "Error during png creation\n");
		code = 1;
		goto finalise;
	}

	png_init_io(png_ptr, fp);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, width, height,
			8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// Set title
	if (title != NULL) {
		png_text title_text;
		title_text.compression = PNG_TEXT_COMPRESSION_NONE;
		title_text.key = "Title";
		title_text.text = title;
		png_set_text(png_ptr, info_ptr, &title_text, 1);
	}

	png_write_info(png_ptr, info_ptr);

	// Write image data
	int y;
	for (y=0 ; y<height ; y++) {
		png_write_row(png_ptr, buffer[y]);
	}

	// End write
	png_write_end(png_ptr, NULL);

	finalise:
	if (fp != NULL) fclose(fp);
	if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

	return code;
}
