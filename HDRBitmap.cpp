#include "precomp.h"

HDRBitmap::HDRBitmap(const char* fileName)
{
	FIBITMAP* dib = FreeImage_Load(FIF_HDR, fileName);

	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);

	this->buffer = new vec4[width * height];

	struct pixel
	{
		float x, y, z;
	};

	for (int y = 0; y < height; y++)
	{
		pixel* line = (pixel*)FreeImage_GetScanLine(dib, height - 1 - y);
		for (int x = 0; x < width; x++)
		{
			buffer[y * width + x] = vec4(line[x].x, line[x].y, line[x].z, 1.0f);
		}
	}
}
