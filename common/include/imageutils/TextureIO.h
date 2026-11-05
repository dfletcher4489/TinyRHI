#pragma once
#include "CommonRenderTypes.h"
#include "OSFile.h"

namespace TexUtils
{
	inline void BGRATexture(char* image, int height, int width, int stride)
	{
		for (int i = 0; i < width * height * stride; i += stride)
		{
			char temp = image[i];
			image[i] = image[i + 2];
			image[i+2] = temp;
		}
	}

	namespace BMP
	{
#pragma pack(push, 1)
		typedef struct BitmapFileHeader
		{
			uint16_t  bfType;
			uint32_t  bfSize;
			uint16_t  bfReserved1;
			uint16_t  bfReserved2;
			uint32_t  bfOffBits;
		} BitmapFileHeader;

		typedef struct BitmapInfoHeader
		{
			uint32_t biSize;
			uint32_t biWidth;
			uint32_t biHeight;
			uint16_t biPlanes;
			uint16_t biBitCount;
			uint32_t biCompression;
			uint32_t biSizeImage;
			uint32_t biXPelsPerMeter;
			uint32_t biYPelsPerMeter;
			uint32_t biClrUsed;
			uint32_t biClrImportant;
		} BitmapInfoHeader;
#pragma pack(pop)

		static_assert(sizeof(BitmapFileHeader) == 14);
		static_assert(sizeof(BitmapInfoHeader) == 40);

		inline void WriteOutBMPHeaders(BitmapFileHeader *fileheader, BitmapInfoHeader *infoheader, uint32_t width, uint32_t height)
		{
			*fileheader = { 0x4d42, (40 + 14 + (width * height * 4)), 0, 0, 0x36 };
			*infoheader = { sizeof(BitmapInfoHeader), width, height, 1, 32, 0, 0, 0, 0, 0, 0 };
		}
	}

	namespace DDS
	{
		enum DDS_FLAGS
		{
			DDSD_CAPS = 0x1,
			DDSD_HEIGHT = 0x2,
			DDSD_WIDTH = 0x4,
			DDSD_PITCH = 0x8,
			DDSD_PIXELFORMAT = 0x1000,
			DDSD_MIPMAPCOUNT = 0x20000,
			DDSD_LINEARSIZE = 0x80000,
			DDSD_DEPTH = 0x800000,
		};

		enum DDS_CAPS1_FLAGS
		{
			DDSCAPS_COMPLEX = 0x8,
			DDSCAPS_MIPMAP = 0x400000,
			DDSCAPS_TEXTURE = 0x1000
		};

		enum DDS_CAPS2_FLAGS
		{
			DDSCAPS2_CUBEMAP = 0x200,
			DDSCAPS2_CUBEMAP_POSITIVEX = 0x400,
			DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800,
			DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000,
			DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000,
			DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000,
			DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000,
			DDSCAPS2_VOLUME = 0x200000
		};

		enum DDS_PIXELFORMAT_FLAGS
		{
			DDPF_ALPHAPIXELS = 0x1,
			DDPF_ALPHA = 0x2,
			DDPF_FOURCC = 0x4,
			DDPF_RGB = 0x40,
			DDPF_YUV = 0x200,
			DDPF_LUMINANCE = 0x20000
		};

		struct DDS_PIXELFORMAT 
		{
			uint32_t dwSize;
			uint32_t dwFlags;
			uint32_t dwFourCC;
			uint32_t dwRGBBitCount;
			uint32_t dwRBitMask;
			uint32_t dwGBitMask;
			uint32_t dwBBitMask;
			uint32_t dwABitMask;
		};

		struct DDS_HEADER
		{
			uint32_t magic;
			uint32_t size;
			uint32_t flags;
			uint32_t height;
			uint32_t width;
			uint32_t pitchOrLinearSize;
			uint32_t depth;
			uint32_t mipMapCount;
			uint32_t reserved[11];
			DDS_PIXELFORMAT ddspf;
			uint32_t caps;
			uint32_t caps2;
			uint32_t caps3;
			uint32_t caps4;
			uint32_t reserved2;
		};

		struct DDS_HEADER_DXT10
		{
			uint32_t dxgiFormat;
			uint32_t resourceDimension;
			uint32_t miscFlag;
			uint32_t arraySize;
			uint32_t miscFlags2;
		};
	}
}

enum TextureIOType
{
	BMP = 0,
	DDS = 1,
};

struct TextureDetails
{
	ImageFormat type;
	uint32_t dataSize;
	uint32_t width, height, miplevels;
	char* data;
	char* currPointer;
	uint32_t arrayLayers;
	uint32_t bitcount;
};


int ReadBMPDetails(char* _fileData, TextureDetails* details);

void ReadBMPData(char* _fileData, int dataPointer, TextureDetails* details);

int ReadDDSDetails(char* _fileData, TextureDetails* details);

void ReadDDSData(char* _fileData, int dataPointer, TextureDetails* details);

int ReadBMPDetailsFromFile(OSFileHandle* fileHandle, TextureDetails* details);
int ReadDDSDetailsFromFile(OSFileHandle* fileHandle, TextureDetails* details);
