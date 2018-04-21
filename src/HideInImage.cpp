#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include <png.h>

#include "HideInImage.h"



static void get_rgba_repr(png_structp png, png_infop info)
{
	png_read_info(png, info);

	png_byte color_type = png_get_color_type(png, info);
	png_byte bit_depth  = png_get_bit_depth(png, info);

	// Read any color_type into 8bit depth, RGBA format.
	// See http://www.libpng.org/pub/png/libpng-manual.txt

	if(bit_depth == 16)
		png_set_strip_16(png);

	// decode to rgb
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

int write_to_file(image* img, const char *filename) 
{

	FILE *fp = fopen(filename, "wb");
	if(!fp) 
		return ERROR_CANNOT_OPEN;

	int err = write_to_stream(img, fp);
	fclose(fp);

	return err;
}

int write_to_stream( image* img, FILE* fp )
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


int insert_data(image* img, const unsigned char* data, size_t data_size, size_t Offset)
{
	//not enough space
	if ((data_size) * 8 > img->height * img->width)
	{
		printf("Not capacity\n");
		return 2;
	}

	unsigned int CurByte = 0;
	unsigned int  CurBit = 7;

	//TODO: rewrite loop 
	//control by bits and bytes
	for (unsigned int CurRow = (!Offset ? 1 : Offset) / img->width; CurRow < img->height; ++CurRow)
	{//not enough space
		png_bytep row = img->bytes[CurRow];
		for (unsigned int CurCol = Offset % img->width; CurCol < img->width && CurByte < data_size; ++CurCol)
		{
		//getting color
			png_bytep px = &(row[CurCol * 4]);
      // Do something awesome for each pixel here...
      //printf("%4d, %4d = RGBA(%3d, %3d, %3d, %3d)\n", x, y, px[0], px[1], px[2], px[3]);

      //bitmap may have unused space
      //TODO: find better solution for this
      

      // transparent pixels

      if (px[3] == 0)
      {
        continue;
      }

      //WRITE TO R
      unsigned char r = px[0];

      //getting the pixel value
      //set last bit to 0
      r = r >> 1 << 1;
      //setting bit
      r |= (data[CurByte] >> CurBit) & 1;

      //setting new value
      px[0] = r;

      //next bit
      if (CurBit == 0)
      {
        CurBit = 7;
        ++CurByte;
      }
      else {
        --CurBit;
      }
    }
  }

  if (CurByte == data_size)
    return 0;
  else
    return 1;
}

int retrieve_data(const image* img, unsigned char* data, size_t bytes_retrieve, size_t Offset)
{

  //change to false 
  if ((bytes_retrieve + Offset) * 8 > img->height * img->width)
    return 2;

  //filling 0
  for (size_t i = 0; i < bytes_retrieve; ++i)
    data[i] = 0;

  unsigned int CurByte = 0;
  unsigned int CurBit = 7;

  
  //TODO: rewrite loop
  for (unsigned int CurRow = (!Offset ? 1 : Offset) / img->width; CurRow < img->height; ++CurRow)
  {
    png_bytep row = img->bytes[CurRow];
    for (unsigned int CurCol = Offset % img->width; CurCol < img->width && CurByte < bytes_retrieve; ++CurCol)
    {
      //getting color
      png_bytep px = &(row[CurCol * 4]);

      //unused space
      if (px[3] == 0)
      {
        continue;
      }

      //getting the pixel value
      unsigned char value = px[0];
      //setting current bit
      data[CurByte] |= (value & 1) << CurBit;

      //next bit
      if (CurBit == 0)
      {
        CurBit = 7;
        ++CurByte;
      }
      else {
        --CurBit;
      }
    }
  }
  if (CurByte == bytes_retrieve)
    return 0;
  else
    return 1;
}