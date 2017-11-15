#pragma once
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;


enum ReturnValues {
	ok = 0,
	NotEnoughSpace,
	NotEnoughSpaceWrited,
	CannotOpenFile,
	CannotCreateFile
};

//TODO: write initializer
//Need initialize GDI+ before using

int Encode(Bitmap& bitmap, const  char* Data, UINT BytesToEncode, UINT Offset = 0);
//make bitmap and pass it to functions above

//Return values
//0 - Ok
//1 - Not enough space
//2 - Not enough space but some data writed (cause - empty space)
//4 - file cannot be opened
//5 - file cannot be created
int Encode(const WCHAR* const InFilename, const WCHAR* OutFilename, const  char* Data, UINT BytesToEncode, UINT Offset = 0);

//Decode
int Decode(Bitmap& bitmap,  char* Data, UINT BytesToEncode, UINT Offset = 0);
int Decode(const WCHAR* const  Filename,  char* Data, UINT BytesToEncode, UINT Offset = 0);