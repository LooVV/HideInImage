#pragma once
#include <windows.h>
#include <gdiplus.h>


enum ReturnValues {
	ok = 0,
	NotEnoughSpace,
	NotEnoughSpaceWrited,
	CannotOpenFile,
	CannotCreateFile
};

//TODO: write initializer
//Need initialize GDI+ before using
//setup gdi, call this before using Encode or decode
ULONG_PTR SetupGdi();
//Gdi shutdown, call this after using functions
VOID GdiShutdown(ULONG_PTR gdiplusToken);

int Encode(Gdiplus::Bitmap& bitmap, const  char* Data, UINT BytesToEncode, UINT Offset = 0);
//make bitmap and pass it to functions above

int Encode(const WCHAR* const InFilename, const WCHAR* OutFilename, const  char* Data, UINT BytesToEncode, UINT Offset = 0);

//Decode
int Decode(Gdiplus::Bitmap& bitmap,  char* Data, UINT BytesToEncode, UINT Offset = 0);
int Decode(const WCHAR* const  Filename,  char* Data, UINT BytesToEncode, UINT Offset = 0);