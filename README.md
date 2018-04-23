# HideInImage
Simle steganography module to store data in image

# Build 

### Windows
  Use HideInImage.vcxproj to build project from Visual Studio

### Linux
  Step inside project directory and run:
  
    make

## Dependencies
  [libpng](https://github.com/glennrp/libpng) - used process .png files. You should specify library paths in Makefile or VC project.
 
# Usage 
Program:

    hide_in_image image load/extract datafilename offset channel(R/G/B)
    
Where:

 * hide_in_image  - executable name
  
 * image          - filename of image
  
 * load / extract - specify operation, if load, add data to image, if extract - fetch
  
 * datafilename   - depending on operation, source of data, that will be writen to image
                   or output data from image
                   
 * offset         - number bits positions from begning of the image
  
 * channel        - channel to write (red, green, blue)
 
### Or you can simply add HideInFile.h and HideInFile.cpp to your code:
    
Functions that you need is __insert_data__ and __retrieve_data__. 

Before using this image must be loaded by functions __load_form_file__ or __load_form_stream__.

To save result call __save_to_file__ or __save_to_stream__.
    
If error occur or work is done release resources, using __release_image__.
  
# Method 
Last significant bit substitution method was used.
