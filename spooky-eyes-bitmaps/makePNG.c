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

#include "defaultEye.h"

// This function actually writes out the PNG image file. The string 'title' is
// also written into the image file
int writeImage(char* filename, int width, int height, png_bytep* buffer, char* title);

png_bytep* allocPNGBuffer(int width, int height)
{
	png_bytep* row_pointers = (png_bytep*)malloc(height*sizeof(png_bytep));
	int row, column;
	for (row=0; row < height; row++)
	{
		row_pointers[row] = (png_bytep)malloc(3*width); // Each pixel has three bytes for RGB
	}

	return row_pointers;
}

void freePNGBuffer(int height, png_bytep* buffer)
{
	for (int row=0; row < height; row++)
	{
		free(buffer[row]);
	}
	free(buffer);
}

int main(int argc, char *argv[])
{
	// Make sure that the output filename argument has been provided
	if (argc != 2) {
		fprintf(stderr, "Please specify output file\n");
		return 1;
	}

	// Specify an output image size
	int width = 500;
	int height = 300;

	// Create a test image
	png_bytep* row_pointers = allocPNGBuffer(width, height);

	int row, column;
	for (row=0; row < height; row++)
	{
		for (column=0; column<width; column++)
		{
			row_pointers[row][column*3] = row % 0xFF;
			row_pointers[row][column*3+1] = 0xCD;
			row_pointers[row][column*3+2] = column % 0xFF;
		}
	}

	// Save the image to a PNG file
	// The 'title' string is stored as part of the PNG file
	printf("Saving PNG\n");
	int result = writeImage(argv[1], width, height, row_pointers, "This is my test image");

	// Free up the memorty used to store the image
	freePNGBuffer(height, row_pointers);

	return result;
}

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
