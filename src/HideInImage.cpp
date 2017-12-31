#include "HideInImage.h"
#pragma comment(lib,"gdiplus.lib")

using namespace Gdiplus;

ULONG_PTR SetupGdi()
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	return gdiplusToken;
}

VOID GdiShutdown(ULONG_PTR gdiplusToken)
{
	GdiplusShutdown(gdiplusToken);
}

static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

int Encode(const WCHAR* InFilename, const WCHAR* OutFilename, const  char* Data, UINT BytesToEncode, UINT Offset)
{
	Bitmap   bitmap{ InFilename };

	if ( GetLastError() )
	{
		return CannotCreateFile;
	}

	int res = Encode(bitmap, Data, BytesToEncode, Offset);
	if (res == 0) {

		CLSID   encoderClsid;
		GetEncoderClsid(L"image/png", &encoderClsid);

		if (bitmap.Save(OutFilename, &encoderClsid) != Ok)
			res = false;
	}
	return res;
}

int Encode(Bitmap& bitmap, const char* Data, UINT BytesToEncode, UINT Offset )
{
	UINT height = bitmap.GetHeight();
	UINT widht = bitmap.GetWidth();

	//not enough space
	if ((Offset + BytesToEncode) * 8 > height * widht)
		return NotEnoughSpace;


	UINT CurByte = 0;
	UINT CurBit = 7;

	Color color;
	Status res;
	//TODO: rewrite loop 
	//control by bits and bytes
	for (UINT CurRow = (!Offset ? 1 : Offset) / widht; CurRow < height; ++CurRow)
	{
		for (UINT CurCol = Offset % widht; CurCol < widht && CurByte < BytesToEncode; ++CurCol)
		{
			//getting color
			res = bitmap.GetPixel(CurRow, CurCol, &color);
			//bitmap may have unused space
			//TODO: find better solution for this
			if (res != Ok)
				continue;

			//getting the pixel value
			UINT value = color.GetValue();
			//set last bit to 0
			value = value >> 1 << 1;
			//setting bit
			value |= (Data[CurByte] >> CurBit) & 1;

			//setting new value
			color.SetValue(value);
			bitmap.SetPixel(CurRow, CurCol, color);

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

	if (CurByte == BytesToEncode)
		return ok;
	else
		return NotEnoughSpaceWrited;
}

int Decode(Bitmap& bitmap,  char* Data, UINT BytesToEncode, UINT Offset )
{
	UINT height = bitmap.GetHeight();
	UINT widht = bitmap.GetWidth();

	//change to false 
	if ((BytesToEncode + Offset) * 8 > height * widht)
		return NotEnoughSpace;

	//filling 0
	for (size_t i = 0; i < BytesToEncode; ++i)
		Data[i] = 0;

	UINT CurByte = 0;
	UINT CurBit = 7;

	Color color;
	Status res;

	//TODO: rewrite loop
	for (UINT CurRow = (!Offset ? 1 : Offset) / widht; CurRow < height; ++CurRow)
	{
		for (UINT CurCol = Offset % widht; CurCol < widht && CurByte < BytesToEncode; ++CurCol)
		{
			//getting color
			res = bitmap.GetPixel(CurRow, CurCol, &color);
			//unused space
			if (res != Ok)
				continue;

			//getting the pixel value
			UINT value = color.GetValue();
			//setting current bit
			Data[CurByte] |= (value & 1) << CurBit;

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
	if (CurByte == BytesToEncode)
		return ok;
	else
		return NotEnoughSpaceWrited;
}


int Decode(const WCHAR* Filename,  char* Data, UINT BytesToEncode, UINT Offset )
{
	Bitmap   bitmap{ Filename };

	if (GetLastError() != Ok)
		return CannotOpenFile;

	return Decode(bitmap, Data, BytesToEncode, Offset);
}

