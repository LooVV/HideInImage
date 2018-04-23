#pragma once

#include <png.h>


enum channels
{
	RED,
	GREEN,
	BLUE
};

enum errors
{
	NO_ERROR,
	ERROR_CANNOT_OPEN,
	ERROR_NOT_PNG,
	ERROR_INTERNAL,
	ERROR_BAD_ALLOC,
	ERROR_NOT_ENOUGH_PIXELS
};

struct image
{
	png_struct* 		image;
	png_info*	 		info_begin;
	png_info*			info_end;
	png_byte** 			bytes;
	unsigned int 		height;
	unsigned int 		width;
};


int insert_data  (image* img, const unsigned char* data, size_t data_size, 		size_t Offset, unsigned int channel);
int retrieve_data(const image* img, unsigned char* data, size_t bytes_retrieve, size_t Offset, unsigned int channel);





/*
		Load image
 */

/* possible errors:
	ERROR_NOT_PNG,
	ERROR_INTERNAL,
	ERROR_BAD_ALLOC
 */
int load_form_file(image* img, const char *filename);

/* possible errors:
	ERROR_CANNOT_OPEN,
	ERROR_NOT_PNG,
	ERROR_INTERNAL,
	ERROR_BAD_ALLOC
*/
int load_form_stream(image* img, FILE* fp);






// release all resources related to image
// call when error occur or after work
void release_image(image* img);





/*
		Saving 
 */



/* possible errors:
	ERROR_CANNOT_OPEN,
	ERROR_INTERNAL
*/
int save_to_stream	( image* img, FILE* fp );

/* possible errors:
	ERROR_INTERNAL
*/
int save_to_file	( image* img, const char *filename); 
