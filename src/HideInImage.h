#pragma once

struct png_struct_def;
struct png_info_def;

enum errors
{
	NO_ERROR,
	ERROR_CANNOT_OPEN,
	ERROR_NOT_PNG,
	ERROR_INTERNAL,
	ERROR_BAD_ALLOC
};

struct image
{
	png_struct_def* image;
	png_info_def* 	info_begin;
	png_info_def* 	info_end;
	unsigned char** 	bytes;
	unsigned int 		height;
	unsigned int 		width;
};


int load_form_file(image* img, const char *filename);
int load_form_stream(image* img, FILE* fp);


void release_image(image* img);



int write_to_stream( image* img, FILE* fp );
int write_to_file(image* img, const char *filename); 

int insert_data(image* img, const unsigned char* data, size_t data_size, size_t Offset);
int retrieve_data(const image* img, unsigned char* data, size_t bytes_retrieve, size_t Offset);




