#pragma once

namespace Tmpl8 {
	class HDRBitmap
	{
	public:
		HDRBitmap(const char* fileName);

		int width, height;
		vec4* buffer;
	};
}

