#include "imageutils/TextureIO.h"
#include <string.h>

void ReadBMPData(char* _fileData, int dataPointer, TextureDetails* details)
{
	int uLine;
	int bottomLine = details->height - 1;
	int bytesPerRow = (details->bitcount>>3) * details->width;
	int i = 0;

	uLine = bottomLine;

	auto fileDataIter = _fileData + (dataPointer + (uLine * bytesPerRow));

	auto textureMemoryCache = details->currPointer;

	for (; i < bottomLine; i++)
	{
		memcpy(textureMemoryCache, fileDataIter, bytesPerRow);
		textureMemoryCache += bytesPerRow;
		fileDataIter -= bytesPerRow;
	}

	memcpy(textureMemoryCache, fileDataIter, bytesPerRow);
}

int ReadBMPDetails(char* _fileData, TextureDetails* details)
{
	char* iter = _fileData;

	TexUtils::BMP::BitmapFileHeader fh;

	memcpy(reinterpret_cast<char*>(&fh), iter, sizeof(TexUtils::BMP::BitmapFileHeader));

	if (fh.bfType != 0x4D42)
	{
		return -1;
	}

	iter += sizeof(TexUtils::BMP::BitmapFileHeader);

	TexUtils::BMP::BitmapInfoHeader ih;

	memcpy(reinterpret_cast<char*>(&ih), iter, sizeof(TexUtils::BMP::BitmapInfoHeader));

	uint32_t width = details->width = ih.biWidth;
	uint32_t height = details->height = ih.biHeight;
	uint32_t bitcount = details->bitcount = ih.biBitCount;

	switch (bitcount)
	{
	case 32:
		details->type = ImageFormat::B8G8R8A8;
		break;
	default:
		return -1;
	}

	if (ih.biSizeImage)
	{
		details->dataSize = ih.biSizeImage;
	}
	else
	{
		details->dataSize = fh.bfSize - fh.bfOffBits;
	}

	details->miplevels = 1;

	return fh.bfOffBits;
}

ImageFormat DXGIToImageFormat(uint32_t dxgiFormat)
{
	switch (dxgiFormat)
	{
	case 28: return ImageFormat::R8G8B8A8_UNORM;      // R8G8B8A8_UNORM
	case 27: return ImageFormat::R8G8B8A8;            // R8G8B8A8_TYPELESS
	case 87: return ImageFormat::B8G8R8A8_UNORM;      // B8G8R8A8_UNORM
	case 90: return ImageFormat::B8G8R8A8;            // B8G8R8A8_TYPELESS
	case 71: return ImageFormat::DXT1;                // BC1_UNORM
	case 74: return ImageFormat::DXT3;                // BC2_UNORM
	case 45: return ImageFormat::D24UNORMS8STENCIL;   // D24_UNORM_S8_UINT
	case 20: return ImageFormat::D32FLOATS8STENCIL;   // D32_FLOAT_S8X24_UINT
	case 40: return ImageFormat::D32FLOAT;            // D32_FLOAT
	case 42: return ImageFormat::R32_UINT;            // R32_UINT
	default: return ImageFormat::IMAGE_UNKNOWN;
	}
}

int ReadDDSDetails(char* _fileData, TextureDetails* details)
{
	char* iter = _fileData;

	TexUtils::DDS::DDS_HEADER header{};

	memcpy(reinterpret_cast<char*>(&header), iter, sizeof(TexUtils::DDS::DDS_HEADER));

	if (header.magic != 0x20534444)
	{
		return -1;
	}

	details->width = header.width;
	details->height = header.height;
	details->miplevels = header.mipMapCount;

	uint32_t dx10HeaderSize = 0;

	iter += header.size + 4;

	if (header.ddspf.dwFlags & TexUtils::DDS::DDPF_FOURCC)
	{
		if (memcmp(&header.ddspf.dwFourCC, "DXT1", sizeof(uint32_t)) == 0)
		{
			details->type = ImageFormat::DXT1;
		}
		else if (memcmp(&header.ddspf.dwFourCC, "DXT3", sizeof(uint32_t)) == 0)
		{
			details->type = ImageFormat::DXT3;
		}
		else if (memcmp(&header.ddspf.dwFourCC, "DX10", sizeof(uint32_t)) == 0)
		{
			dx10HeaderSize = sizeof(TexUtils::DDS::DDS_HEADER_DXT10);

			TexUtils::DDS::DDS_HEADER_DXT10 dx10header{};

			memcpy(&dx10header, iter, dx10HeaderSize);

			details->type = DXGIToImageFormat(dx10header.dxgiFormat);
		}
	} 
	
	if (header.ddspf.dwFlags & TexUtils::DDS::DDPF_RGB)
	{
		details->bitcount = header.ddspf.dwRGBBitCount;

		if (header.ddspf.dwFlags & TexUtils::DDS::DDPF_ALPHAPIXELS)
		{
			if ((header.ddspf.dwRBitMask == 255) && (header.ddspf.dwGBitMask == 65280) && (header.ddspf.dwBBitMask == 16711680) && (header.ddspf.dwABitMask == 4278190080))
			{
				details->type = ImageFormat::R8G8B8A8_UNORM;
			}
			else if ((header.ddspf.dwBBitMask == 255) && (header.ddspf.dwGBitMask == 65280) && (header.ddspf.dwRBitMask == 16711680) && (header.ddspf.dwABitMask == 4278190080))
			{
				details->type = ImageFormat::B8G8R8A8_UNORM;
			}
		}
	}

	details->arrayLayers = 1;

	uint32_t caps2 = header.caps2;

	if (caps2 & TexUtils::DDS::DDSCAPS2_CUBEMAP)
	{
		details->arrayLayers = 6;
	}

	if (details->type == ImageFormat::DXT1 || details->type == ImageFormat::DXT3) 
	{
		details->dataSize = header.pitchOrLinearSize;
	}
	else if (details->type == ImageFormat::R8G8B8A8_UNORM || details->type == ImageFormat::B8G8R8A8_UNORM)
	{
		details->dataSize = (details->height * header.pitchOrLinearSize) * details->arrayLayers;
	}
	
	return header.size + 4 + dx10HeaderSize;
}

void ReadDDSData(char* _fileData, int dataPointer, TextureDetails* details)
{
	memcpy(details->data, (_fileData + dataPointer), details->dataSize);	
}