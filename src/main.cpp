#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <ctype.h>
#include <string>

#include "HideInImage.h"

#define BAD_ARG(arg) \
printf("Unrecognized option: %s\n", arg );\
return 1


const char* err_desc[] = 
{
	"",
	"image file cannot be opened",
	"is not png image",
	"internal error",
	"memory allocation error",
	"size of image too small to contain data"
};


void usage()
{
	printf("Usage: hide_in_image image load/extract datafilename offset channel(R/G/B)\n");
}

bool all_digits(const char* str)
{
	while(*str)
	{
		if( !isdigit(*str))
			return false;
		++str;
	}
	return true;
}




int main(int argc, char* argv[]) 
{
	if( argc != 6 )
	{
		usage();
		return 1;
	}

	char* image_file = argv[1];
	bool load = true;
	
	if( strcmp(argv[2],"load") == 0 )
		load = true;
	else if( strcmp(argv[2],"extract") == 0)
		load = false;
	else {
		BAD_ARG( argv[2] );
	}

	char* data_file  = argv[3];

	
	if( ! all_digits(argv[4]) ){
		BAD_ARG(argv[4]);
	}

	unsigned offset  = atoi(argv[4]);

	if( strlen(argv[5]) != 1){
		BAD_ARG(argv[5]);
	}
	unsigned channel = RED;

	switch(argv[5][0])
	{
	case 'R':
		channel = RED;
		break;
	case 'G':
		channel = GREEN;
		break;
	case 'B':
		channel = BLUE;
		break;
	
	default:
		BAD_ARG(argv[5]);
	}






	image img = {};
	int err =  load_form_file(&img, image_file);
	if( err ){
		printf("Image %s cannot be opened\n", image_file );
		return 1;
	}

	unsigned load_data_size = ( img.height * img.width - offset) / 8 ;


	unsigned char* buff = (unsigned char*)malloc(load_data_size);
	if( ! buff){
		printf("%s\n",err_desc[4]);
		return 1;
	}


	if( load )
	{
		FILE* f = fopen(data_file, "rb");
		if( !f ){
			printf("Cannot open file %s\n",data_file);
			return 1;
		}
		unsigned int actual_size = fread(buff,1, load_data_size, f);

		fclose(f);

		int err = insert_data(&img, buff, actual_size, offset, channel);
		if( err ){
			printf("Data insertion erorr %s\n",err_desc[err] );
			return 1;
		}

	
		// TODO: generate name differently
		err = save_to_file(&img, (std::string("changed_") + image_file).c_str());
		if( err ){
			printf("Saving error %s\n",err_desc[err] );
			return 1;
		}
	}
	// extractint info
	else
	{
		FILE* f = fopen(data_file, "wb");
		if( !f ){
			printf("Cannot open file %s\n",data_file );
			return 1;
		}

		int err = retrieve_data(&img, buff, load_data_size, offset, channel);
		if( err ){
			printf("Extraction error %s\n",err_desc[err]);
			return 1;
		}
			

		if( load_data_size != fwrite( buff, 1, load_data_size, f)){
			printf("Writing error\n" );
			return 1;
		}

		fclose(f);
	}
	free( buff );
}