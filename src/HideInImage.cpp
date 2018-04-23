#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "HideInImage.h"



static void get_rgba_repr(png_structp png, png_infop info)
{
	png_read_info(png, info);

	png_byte color_type = png_get_color_type(png, info);
	png_byte bit_depth  = png_get_bit_depth(png, info);


	if(bit_depth == 16)
		png_set_strip_16(png);

	if(color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);

	// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png);

	if(png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);

	// These color_type don't have an alpha channel then fill it with 0xff.
	if(color_type == PNG_COLOR_TYPE_RGB ||
			color_type == PNG_COLOR_TYPE_GRAY ||
			color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

	if(color_type == PNG_COLOR_TYPE_GRAY ||
			color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	png_read_update_info(png, info);
}

static int load_data(image* img)
{
	img->width  = png_get_image_width (img->image, img->info_begin);
	img->height = png_get_image_height(img->image, img->info_begin);


	img->bytes  = (png_bytep*)malloc(sizeof(png_bytep) * img->height);
	if(! img->bytes )
		return ERROR_BAD_ALLOC;

	for(unsigned i = 0; i < img->height; i++) 
	{
		img->bytes[i] = (png_byte*)malloc(png_get_rowbytes(img->image,img->info_begin));
		if(! img->bytes[i] )
			return ERROR_BAD_ALLOC;
	}
	return NO_ERROR;
}

int load_form_file(image* img,const char *filename) 
{
	FILE *fp = fopen(filename, "rb");

	if(!fp )
 		return ERROR_CANNOT_OPEN;

	int err = load_form_stream(img, fp);

	fclose(fp);
	return err;
}



int load_form_stream(image* img, FILE* fp)
{
	unsigned char header[8];    

	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8))
		return ERROR_NOT_PNG; 

	fseek(fp, -8, SEEK_CUR);


	img->image  = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!img->image)
		return ERROR_INTERNAL;

	img->info_begin = png_create_info_struct(img->image);
	if(!img->info_begin) 
		return ERROR_INTERNAL;
	
	img->info_end = png_create_info_struct(img->image);
	if(!img->info_end) 
		return ERROR_INTERNAL;
	
	if (setjmp(png_jmpbuf(img->image)))
		return ERROR_INTERNAL;
	
	// read image
	png_init_io(img->image, fp);

	get_rgba_repr(img->image, img->info_begin);
	
	int err = load_data(img);
	if( err != NO_ERROR )
		return err;

	png_read_image(img->image, img->bytes);

	return NO_ERROR;
}


void release_image(image* img)
{
	if(img->bytes)
	{
		for(unsigned i = 0; i < img->height && img->bytes[i]; ++i)
			free(img->bytes[i]);

		free(img->bytes);
		img->bytes = NULL;
	}
	if( img->image || img->info_begin || img->info_end )
	{
		png_destroy_read_struct(&(img->image), &(img->info_begin), &(img->info_end));
		img->image 		= NULL;
		img->info_begin = NULL;
		img->info_end 	= NULL;
	}
	img->height = 0;
	img->width 	= 0;
}

int save_to_file(image* img, const char *filename) 
{

	FILE *fp = fopen(filename, "wb");
	if(!fp) 
		return ERROR_CANNOT_OPEN;

	int err = save_to_stream(img, fp);
	fclose(fp);

	return err;
}



int save_to_stream( image* img, FILE* fp )
{
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) 
		return ERROR_INTERNAL;

	png_infop info = png_create_info_struct(png);
	if (!info) 
	{
		png_destroy_read_struct(&png, NULL, NULL);
		return ERROR_INTERNAL;
	}

  
	png_init_io(png, fp);

	// Output is 8bit depth, RGBA format.
	png_set_IHDR(
		png,
		info,
		img->width, img->height,
		8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);
	png_write_info(png, info);

	if (setjmp(png_jmpbuf(png)))
	{
		png_destroy_read_struct(&png, &info, NULL);
		return ERROR_INTERNAL;
	}

	// To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
	// Use png_set_filler().
	// png_set_filler(png, 0, PNG_FILLER_AFTER);

	png_write_image(png, img->bytes);
	png_write_end(png, NULL);
	return NO_ERROR;
}


int insert_data(image* img, const unsigned char* data, size_t data_size, 
				size_t offset, unsigned int channel)
{
	//not enough space
	if ( data_size  * 8 > img->height * img->width + offset	)
		return ERROR_NOT_ENOUGH_PIXELS;

	unsigned int cur_byte = 0;
	unsigned int  cur_bit = 7;

	bool first_in_loop = true;
	for (unsigned int cur_row = (!offset ? 1 : offset) / img->width; cur_row < img->height; ++cur_row)
	{
		png_bytep row = img->bytes[cur_row];
		for (unsigned int cur_col = (first_in_loop ? offset % img->width : 0);
					 cur_col < img->width && cur_byte < data_size; ++cur_col)
		{
			//getting color
			png_bytep px = &(row[cur_col * 4]);

			//getting the pixel value
			unsigned char r = px[channel];

			//set last bit to 0
			r = r >> 1 << 1;
			//setting bit
			r |= (data[cur_byte] >> cur_bit) & 1;

			//setting new value
			px[channel] = r;

			//next bit
			if (cur_bit == 0)
			{
				cur_bit = 7;
				++cur_byte;
			}
			else 
				--cur_bit;
		}
		first_in_loop = false;
	}
	return NO_ERROR;
}

int retrieve_data(const image* img, unsigned char* data, size_t bytes_retrieve,
					size_t offset, unsigned int channel)
{
  	//change to false 
	if ( bytes_retrieve * 8 > img->height * img->width + offset)
		return ERROR_NOT_ENOUGH_PIXELS;


	// fill all zero
	memset(data,0,bytes_retrieve);	

	unsigned int cur_byte = 0;
	unsigned int cur_bit = 7;

  
	bool first_in_loop = true;
	for (unsigned int cur_row = (!offset ? 1 : offset) / img->width; cur_row < img->height; ++cur_row)
	{
		png_bytep row = img->bytes[cur_row];
		for (unsigned int cur_col = (first_in_loop ? offset % img->width : 0);
					 cur_col < img->width && cur_byte < bytes_retrieve; ++cur_col)
		{
			//getting color
			png_bytep px = &(row[cur_col * 4]);

			//getting the pixel value
			unsigned char value = px[channel];
			//setting current bit
			data[cur_byte] |= (value & 1) << cur_bit;

			//next bit
			if (cur_bit == 0)
			{
				cur_bit = 7;
				++cur_byte;
			}
			else 
				--cur_bit;
		}
		first_in_loop = false;
	}

	return NO_ERROR;
}