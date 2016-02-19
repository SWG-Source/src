// ======================================================================
//
// TargaFormat.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "sharedImage/FirstSharedImage.h"
#include "sharedImage/TargaFormat.h"
#include "sharedImage/Image.h"

#include "sharedFile/TreeFile.h"

#include <stdio.h>
#include <malloc.h>

#if defined(PLATFORM_LINUX)
#include <alloca.h>
#endif

// ======================================================================

#ifndef TARGA_EVEN_PIXEL_EXPANSION
#define TARGA_EVEN_PIXEL_EXPANSION 0
#endif

#define TARGA_SIGNATURE_LENGTH 18

// ======================================================================

#if defined(PLATFORM_WIN32)

#define PACKING_END_STRUCT

#elif defined(PLATFORM_LINUX)

#define PACKING_END_STRUCT __attribute__ ((packed))

#else

#error figure out how to handle structure packing for this platform

#endif

namespace TargaFormatNamespace
{
	enum TargaImageTypes
	{
		 IT_NoImageData                 = 0
		,IT_UncompressedColorMapped     = 1
		,IT_UncompressedTrueColor       = 2
		,IT_UncompressedBlackAndWhite   = 3
		,IT_RleColorMapped              = 9
		,IT_RleTrueColor                =10
		,IT_RleBlackAndWhite            =11
	};

	const char ms_TargaSignature[TARGA_SIGNATURE_LENGTH] = { 'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.',0 };

#ifdef PLATFORM_WIN32
#pragma pack(push, 1)
#endif

	struct TargaHeader
	{
		uint8  m_idLength;
		uint8  m_colorMapType;
		uint8  m_imageType;
		uint16 m_colorMapStart;
		uint16 m_colorMapLength;
		uint8  m_colorMapDepth;
		uint16 m_xOriginOffset;
		uint16 m_yOriginOffset;
		uint16 m_width;
		uint16 m_height;
		uint8  m_pixelDepth;
		uint8  m_imageDescriptor;
	} PACKING_END_STRUCT;

	struct TargaFooter
	{
		uint32 m_extensionAreaOffset;
		uint32 m_developerDirectoryOffset;
		char   m_signature[TARGA_SIGNATURE_LENGTH];
	} PACKING_END_STRUCT;

#ifdef _WIN32
#pragma pack(pop)
#endif

	static bool _loadImage(AbstractFile *, Image **image, Image::PixelFormat format=Image::PF_nonStandard);

	static void _readUncompressedColorMapped1(
		uint8 *destData,
		Image::PixelFormat destPixelFormat,
		AbstractFile *fileInterface,
		int width,
		int yStart, 
		int yStop, 
		int yStep, 
		int yStride,
		uint8 *cmap,
		int bytesPerColorEntry
	);
	static void _readUncompressedColorMapped2(
		uint8 *destData,
		Image::PixelFormat destPixelFormat,
		AbstractFile *fileInterface,
		int width,
		int yStart, 
		int yStop, 
		int yStep, 
		int yStride,
		uint8 *cmap,
		int bytesPerColorEntry
	);
	static void _readUncompressedBlackAndWhite(
		uint8 *destData,
		Image::PixelFormat destPixelFormat,
		AbstractFile *fileInterface,
		int width,
		int yStart, 
		int yStop, 
		int yStep, 
		int yStride
	);
	static void _readUncompressedTrueColor3(
		uint8 *const destData,
		const Image::PixelFormat destPixelFormat,
		AbstractFile *const fileInterface,
		const int width,
		const int yStart, 
		const int yStop, 
		const int yStep, 
		const int yStride
	);
	static void _readUncompressedTrueColor4(
		uint8 *const destData,
		const Image::PixelFormat destPixelFormat,
		AbstractFile *const fileInterface,
		const int width,
		const int yStart, 
		const int yStop, 
		const int yStep, 
		const int yStride
	);
	static void _readRleColorMapped(
		uint8 *destData,
		Image::PixelFormat destPixelFormat,
		AbstractFile *fileInterface,
		int width,
		int yStart, 
		int yStop, 
		int yStep, 
		int yStride,
		uint8 *cmap,
		int bytesPerColorEntry,
		int fileBytesPerPixel
	);
	static void _readRleTrueColor(
		uint8 *destData,
		Image::PixelFormat destPixelFormat,
		AbstractFile *fileInterface,
		int width,
		int yStart, 
		int yStop, 
		int yStep, 
		int yStride,
		int fileBytesPerPixel
	);
}

using namespace TargaFormatNamespace;
// ======================================================================

TargaFormat::TargaFormat()
:
	ImageFormat()
{
}

// ----------------------------------------------------------------------

TargaFormat::~TargaFormat()
{
}

// ----------------------------------------------------------------------

const char *TargaFormat::getName() const
{
	return "Targa";
}

// ----------------------------------------------------------------------

bool TargaFormat::isValidImage(const char *filename) const
{
	if (!filename || !*filename)
	{
		REPORT_LOG(true, ("TargaFormat::isValidImage(): bad filename\n"));
		return false;
	}

	// -TRF- to do
	DEBUG_REPORT_LOG(true, ("-TRF- accessing TargaFormat::isValidImage(), not yet implemented (returning false)\n"));
	return false; //lint !e527 // unreachable // yes
}

// ----------------------------------------------------------------------

bool TargaFormat::loadImage(const char *filename, Image **image) const
{
	//-- validate args
	if (!filename || !*filename)
	{
		REPORT_LOG(true, ("TargaFormat::loadImage(): bad filename\n"));
		return false;
	}

	if (!image)
	{
		REPORT_LOG(true, ("TargaFormat::loadImage(): nullptr image pointer\n"));
		return false;
	}
	*image            = 0;

	//-- open file
	AbstractFile *fileInterface = TreeFile::open(filename, AbstractFile::PriorityData, true);
	if(!fileInterface)
	{
		REPORT_LOG(true, ("TargaFormat failed to open file [%s]\n", filename));
		return false;
	}

	bool result = _loadImage(fileInterface, image);

	delete fileInterface;
	return result;
}


// ----------------------------------------------------------------------

bool TargaFormat::loadImageReformat(const char *filename, Image **image, Image::PixelFormat format) const
{
	//-- validate args
	if (!filename || !*filename)
	{
		REPORT_LOG(true, ("TargaFormat::loadImage(): bad filename\n"));
		return false;
	}

	if (!image)
	{
		REPORT_LOG(true, ("TargaFormat::loadImage(): nullptr image pointer\n"));
		return false;
	}
	*image            = 0;

	//-- open file
	AbstractFile *fileInterface = TreeFile::open(filename, AbstractFile::PriorityData, true);
	if(!fileInterface)
	{
		REPORT_LOG(true, ("TargaFormat failed to open file [%s]\n", filename));
		return false;
	}

	bool result=_loadImage(fileInterface, image, format);

	REPORT_LOG(!result, ("Error reading Targa: %s\n", filename));

	delete fileInterface;
	return result;
}

// ----------------------------------------------------------------------

bool TargaFormatNamespace::_loadImage(AbstractFile *const fileInterface, Image **image, Image::PixelFormat imageFormat)
{
	//------------------------------------------
   // File header
	TargaHeader header;
	if (fileInterface->read(&header, sizeof(header))!=sizeof(header))
	{
		REPORT_LOG(true, ("TargaFormat failed to read file header.\n"));
		return false;
	}

   if (	header.m_imageType!=IT_UncompressedColorMapped
		&& header.m_imageType!=IT_UncompressedTrueColor
		&& header.m_imageType!=IT_UncompressedBlackAndWhite
		&& header.m_imageType!=IT_RleColorMapped
		&& header.m_imageType!=IT_RleTrueColor
	)
   {
      // No code here to convert them, so fail
		REPORT_LOG(true, ("TargaFormat found unsupported image format.\n"));
      return false;
   }
	//------------------------------------------

	//------------------------------------------
	// Image ID
	const int imageIDOffset = sizeof(header);
	//------------------------------------------

	//------------------------------------------
	// Color Map Data (field 7)
	const int colorMapFileOffset = imageIDOffset + static_cast<int>(header.m_idLength);

	if (header.m_colorMapStart!=0 || (header.m_colorMapDepth&0x7)!=0)
	{
		REPORT_LOG(true, ("TargaFormat found unsupported color-map data.\n"));
		return false;
	}

	const int colorMapLength     = header.m_colorMapLength;
	const int bytesPerColorEntry =(header.m_colorMapDepth>>3);
	const int colorMapSize       = colorMapLength*bytesPerColorEntry;

	uint8 *cmap=0;
	if (colorMapSize > 0)
	{
		if (!fileInterface->seek(AbstractFile::SeekBegin, colorMapFileOffset))
		{
			REPORT_LOG(true, ("TargaFormat failed to read color-map data.\n"));
			return false;
		}

		cmap = (uint8 *)alloca(colorMapSize);
		if (fileInterface->read(cmap, colorMapSize)!=colorMapSize)
		{
			REPORT_LOG(true, ("TargaFormat failed to read color-map data.\n"));
			return false;
		}
	}
	//------------------------------------------

	//------------------------------------------
	// Image Data (field 8) (part 1)

	// seek to image data
	const int imageDataFileOffset = colorMapFileOffset + colorMapSize;
	if (!fileInterface->seek(AbstractFile::SeekBegin, imageDataFileOffset))
	{
		REPORT_LOG(true, ("TargaFormat failed to read image pixel data.\n"));
		return false;
	}
	//------------------------------------------

	//------------------------------------------
   // analyze file pixel format load pixel format.
   bool isTopToBottom;
	Image::PixelFormat filePixelFormat; 
   int attributeBits=0;
	int diskBytesPerPixel=0;

   // file pixel format
   attributeBits = header.m_imageDescriptor&0xf;
   if (attributeBits!=0 && attributeBits!=8)
   {
		REPORT_LOG(true, ("TargaFormat found unsupported attribute channel.\n"));
      return false;
   }

   isTopToBottom=(header.m_imageDescriptor&0x20)!=0;

   filePixelFormat=Image::PF_nonStandard;
	diskBytesPerPixel=header.m_pixelDepth>>3;

   switch (header.m_imageType)
   {
   case IT_UncompressedColorMapped:
	case IT_RleColorMapped:
      if (bytesPerColorEntry==3)
		{
         filePixelFormat=Image::PF_bgr_888;
		}
      else if (bytesPerColorEntry==4)
		{
         filePixelFormat=Image::PF_bgra_8888;
		}
      break;

   case IT_UncompressedBlackAndWhite:
      if (header.m_pixelDepth==8)
		{
         filePixelFormat=Image::PF_w_8;
		}
      break;

   case IT_UncompressedTrueColor:
   case IT_RleTrueColor:
      switch (header.m_pixelDepth)
      {
      case 24:
         if (attributeBits==0)
			{
            filePixelFormat=Image::PF_bgr_888;
			}
         else if (attributeBits==8)
			{
            filePixelFormat=Image::PF_bgra_8888;
				diskBytesPerPixel=4;
			}
         break;
      case 32:
         filePixelFormat=Image::PF_bgra_8888;
         break;
      }
      break;
   };

   if (filePixelFormat==Image::PF_nonStandard)
   {
		REPORT_LOG(true, ("TargaFormat found unsupported image pixel format.\n"));
      return false;
   }
	//------------------------------------------

	//------------------------------------------
   // create destination image

	// allocate new image object.
	Image *newImage = NON_NULL(new Image());

	// set image dimensions and pixel format
	const int width                          = static_cast<int>(header.m_width);
	const int height                         = static_cast<int>(header.m_height);
	int destBytesPerPixel=0;

	const Image::PixelFormat destPixelFormat = (imageFormat==Image::PF_nonStandard) ? filePixelFormat : imageFormat;
	switch (destPixelFormat)
	{
	case Image::PF_bgr_888:
		destBytesPerPixel=3;
		newImage->setPixelInformation(0x00ff0000,0x0000ff00,0x000000ff,0x00000000);
		break;
	case Image::PF_bgra_8888:
		destBytesPerPixel=4;
		newImage->setPixelInformation(0x00ff0000,0x0000ff00,0x000000ff,0xff000000);
		break;
	case Image::PF_w_8:
		destBytesPerPixel=1;
		newImage->setPixelInformation(0,0,0,0);
		break;
	default:
		DEBUG_FATAL(true, ("Need to set pixel information for destination pixel format.\n"));
	}
	newImage->setDimensions(width, height, destBytesPerPixel*8, destBytesPerPixel);

	// fill in image data
	uint8 *destData = NON_NULL(newImage->lock());
	//------------------------------------------

	//------------------------------------------
	// Image Data (field 8) (part 2)
   int yStart, yStop, yStep, yStride;

   yStride=newImage->getStride();

   if (isTopToBottom)
   {
      yStart=0;
      yStop=height;
      yStep=1;
   }
   else
   {
      yStart=height-1;
      yStop=-1;
      yStep=-1;
   }

   switch (header.m_imageType)
   {
   case IT_UncompressedColorMapped:
	{
		switch (diskBytesPerPixel)
		{
		case 1:
		{
			_readUncompressedColorMapped1(
				destData, 
				destPixelFormat, 
				fileInterface,
				width,
				yStart, 
				yStop, 
				yStep, 
				yStride,
				cmap,
				bytesPerColorEntry
				);
		} break;
		case 2:
			_readUncompressedColorMapped2(
				destData, 
				destPixelFormat, 
				fileInterface,
				width,
				yStart, 
				yStop, 
				yStep, 
				yStride,
				cmap,
				bytesPerColorEntry
				);
			break;

		default: DEBUG_FATAL(true, ("TargaFormat: Unhandled diskBytesPerPixel.\n"));
		}
   } break;

   case IT_UncompressedBlackAndWhite:
	{
		_readUncompressedBlackAndWhite(
			destData,
			destPixelFormat,
			fileInterface,
			width,
			yStart, 
			yStop, 
			yStep, 
			yStride
		);
	} break;

   case IT_UncompressedTrueColor:
      switch (diskBytesPerPixel)
      {
      case 3:
			_readUncompressedTrueColor3(
				destData,
				destPixelFormat,
				fileInterface,
				width,
				yStart, 
				yStop, 
				yStep, 
				yStride
			);
      break;

      case 4:
			_readUncompressedTrueColor4(
				destData,
				destPixelFormat,
				fileInterface,
				width,
				yStart, 
				yStop, 
				yStep, 
				yStride
			);
      break;
      }
   break;

   case IT_RleColorMapped:
		_readRleColorMapped(
			destData, 
			destPixelFormat, 
			fileInterface,
			width,
			yStart, 
			yStop, 
			yStep, 
			yStride,
			cmap,
			bytesPerColorEntry,
			diskBytesPerPixel
			);
   break;

   case IT_RleTrueColor:
		_readRleTrueColor(
			destData, 
			destPixelFormat, 
			fileInterface,
			width,
			yStart, 
			yStop, 
			yStep, 
			yStride,
			diskBytesPerPixel
			);
	break;
   }
	//------------------------------------------

	//------------------------------------------
	// Finish up.
	newImage->unlock();
	*image   = newImage;
	newImage = 0;

	// no errors
	return true;
}

// ----------------------------------------------------------------------

void TargaFormatNamespace::_readUncompressedColorMapped1(
	uint8 *const destData,
	const Image::PixelFormat destPixelFormat,
	AbstractFile *const fileInterface,
	const int width,
	const int yStart, 
	const int yStop, 
	const int yStep, 
	const int yStride,
	uint8 *const cmap,
	const int bytesPerColorEntry
)
{
	uint8 red=0, green=0, blue=0, alpha=0;
	uint8 *lookup;
	const int lineSize = width;
	uint8 *line = (uint8 *)alloca(lineSize);

	switch (bytesPerColorEntry)
	{
	case 1:
	{
		for (int y=yStart;y!=yStop;y+=yStep)
		{
			uint8 *destIter = destData + y*yStride;
			fileInterface->read(line, lineSize);
			for (int x=0;x<width;x++)
			{
				lookup = cmap + int(line[x]);
				red  =lookup[0];
				green=lookup[0];
				blue =lookup[0];
				alpha=255;
				Image::setPixel(destIter, destPixelFormat, red, green, blue, alpha);
			}
		}
	} break;
	case 3:
	{
		for (int y=yStart;y!=yStop;y+=yStep)
		{
			uint8 *destIter = destData + y*yStride;
			fileInterface->read(line, lineSize);
			for (int x=0;x<width;x++)
			{
				lookup = cmap + int(line[x])*3;
				red  =lookup[0];
				green=lookup[1];
				blue =lookup[2];
				alpha=255;
				Image::setPixel(destIter, destPixelFormat, red, green, blue, alpha);
			}
		}
	} break;
	case 4:
	{
		for (int y=yStart;y!=yStop;y+=yStep)
		{
			uint8 *destIter = destData + y*yStride;
			fileInterface->read(line, lineSize);
			for (int x=0;x<width;x++)
			{
				lookup = cmap + int(line[x])*4;
				red  =lookup[0];
				green=lookup[1];
				blue =lookup[2];
				alpha=lookup[3];
				Image::setPixel(destIter, destPixelFormat, red, green, blue, alpha);
			}
		}
	} break;
	}
}

// ----------------------------------------------------------------------

void TargaFormatNamespace::_readUncompressedColorMapped2(
	uint8 *const destData,
	const Image::PixelFormat destPixelFormat,
	AbstractFile *const fileInterface,
	const int width,
	const int yStart, 
	const int yStop, 
	const int yStep, 
	const int yStride,
	uint8 *const cmap,
	const int bytesPerColorEntry
)
{
	uint8 red=0, green=0, blue=0, alpha=0;
	uint8 *lookup;
	const int lineSize = width * 2;
	uint16 *line = (uint16 *)alloca(lineSize);

	switch (bytesPerColorEntry)
	{
	case 1:
	{
		for (int y=yStart;y!=yStop;y+=yStep)
		{
			uint8 *destIter = destData + y*yStride;
			fileInterface->read(line, lineSize);
			for (int x=0;x<width;x++)
			{
				lookup = cmap + line[x];
				red  =lookup[0];
				green=lookup[0];
				blue =lookup[0];
				alpha=255;
				Image::setPixel(destIter, destPixelFormat, red, green, blue, alpha);
			}
		}
	} break;
	case 3:
	{
		for (int y=yStart;y!=yStop;y+=yStep)
		{
			uint8 *destIter = destData + y*yStride;
			fileInterface->read(line, lineSize);
			for (int x=0;x<width;x++)
			{
				lookup = cmap + int(line[x])*3;
				red  =lookup[0];
				green=lookup[1];
				blue =lookup[2];
				alpha=255;
				Image::setPixel(destIter, destPixelFormat, red, green, blue, alpha);
			}
		}
	} break;
	case 4:
	{
		for (int y=yStart;y!=yStop;y+=yStep)
		{
			uint8 *destIter = destData + y*yStride;
			fileInterface->read(line, lineSize);
			for (int x=0;x<width;x++)
			{
				lookup = cmap + int(line[x])*4;
				red  =lookup[0];
				green=lookup[1];
				blue =lookup[2];
				alpha=lookup[3];
				Image::setPixel(destIter, destPixelFormat, red, green, blue, alpha);
			}
		}
	} break;
	}
}

// ----------------------------------------------------------------------

void TargaFormatNamespace::_readUncompressedBlackAndWhite(
	uint8 *const destData,
	const Image::PixelFormat destPixelFormat,
	AbstractFile *const fileInterface,
	const int width,
	const int yStart, 
	const int yStop, 
	const int yStep, 
	const int yStride
)
{
	uint8 red=0, green=0, blue=0, alpha=0;
	const int lineSize = width;
	uint8 *line = (uint8 *)alloca(lineSize);

   for (int y=yStart;y!=yStop;y+=yStep)
   {
      uint8 *destIter = destData + y*yStride;
		fileInterface->read(line, lineSize);
      for (int x=0;x<width;x++)
      {
         uint8 l = line[x];
         red  =l;
         green=l;
         blue =l;
         alpha=255;

			Image::setPixel(destIter, destPixelFormat, red, green, blue, alpha);
      }
   }
}

// ----------------------------------------------------------------------

void TargaFormatNamespace::_readUncompressedTrueColor3(
	uint8 *const destData,
	const Image::PixelFormat destPixelFormat,
	AbstractFile *const fileInterface,
	const int width,
	const int yStart, 
	const int yStop, 
	const int yStep, 
	const int yStride
)
{
	const int lineSize = width * 3;
	if (destPixelFormat==Image::PF_bgr_888)
	{
		for (int y=yStart;y!=yStop;y+=yStep)
		{
			uint8 *destIter = destData + y*yStride;
			fileInterface->read(destIter, lineSize);
		}
	}
	else
	{
		uint8 red=0, green=0, blue=0, alpha=0;
		uint8 *line = (uint8 *)alloca(lineSize);

		for (int y=yStart;y!=yStop;y+=yStep)
		{
			uint8 *destIter = destData + y*yStride;
			fileInterface->read(line, lineSize);
			for (int x=0;x<width;x++)
			{
				uint8 *p = line + 3*x;
				red  =p[2];
				green=p[1];
				blue =p[0];
				alpha=255;

				Image::setPixel(destIter, destPixelFormat, red, green, blue, alpha);
			}
		}
	}
}

// ----------------------------------------------------------------------

void TargaFormatNamespace::_readUncompressedTrueColor4(
	uint8 *const destData,
	const Image::PixelFormat destPixelFormat,
	AbstractFile *const fileInterface,
	const int width,
	const int yStart, 
	const int yStop, 
	const int yStep, 
	const int yStride
)
{
	const int lineSize = width*4;
	if (destPixelFormat==Image::PF_bgra_8888)
	{
		for (int y=yStart;y!=yStop;y+=yStep)
		{
			uint8 *destIter = destData + y*yStride;
			fileInterface->read(destIter, lineSize);
		}
	}
	else
	{
		uint8 red=0, green=0, blue=0, alpha=0;
		uint8 *line = (uint8 *)alloca(lineSize);

		for (int y=yStart;y!=yStop;y+=yStep)
		{
			uint8 *destIter = destData + y*yStride;
			fileInterface->read(line, lineSize);
			for (int x=0;x<width;x++)
			{
				uint8 *p = line + 4*x;
				red  =p[2];
				green=p[1];
				blue =p[0];
				alpha=p[3];

				Image::setPixel(destIter, destPixelFormat, red, green, blue, alpha);
			}
		}
	}
}

// ----------------------------------------------------------------------

void TargaFormatNamespace::_readRleColorMapped(
	uint8 *destData,
	Image::PixelFormat destPixelFormat,
	AbstractFile *fileInterface,
	int width,
	int yStart, 
	int yStop, 
	int yStep, 
	int yStride,
	uint8 *cmap,
	int bytesPerColorEntry,
	int fileBytesPerPixel
)
{
	uint8 red=0, green=0, blue=0, alpha=0;
	const int lineSize = width*fileBytesPerPixel;
	uint8 *line = (uint8 *)alloca(lineSize);
	uint8 rle;
	uint8 *lookup;
	unsigned index;

	for (int y=yStart;y!=yStop;y+=yStep)
	{
		uint8 *destIter = destData + y*yStride;
		int x=0;
		while (x<width)
		{
	      fileInterface->read(&rle, 1);
			if (rle&0x80) // run length packet
			{
				rle-=127;
				x+=rle;
				DEBUG_FATAL(x>width, ("TargaFormat: Read invalid RLE packet.\n"));

				index=0;
				fileInterface->read(&index, fileBytesPerPixel);
				lookup = cmap + index*bytesPerColorEntry;
				switch (bytesPerColorEntry)
				{
				case 1:
				{
					red  =lookup[0];
					green=lookup[0];
					blue =lookup[0];
					alpha=255;
				} break;
				case 3:
				{
					red  =lookup[0];
					green=lookup[1];
					blue =lookup[2];
					alpha=255;
				} break;
				case 4:
				{
					red  =lookup[0];
					green=lookup[1];
					blue =lookup[2];
					alpha=lookup[3];
				} break;
				}

				while (rle)
				{
					Image::setPixel(destIter, destPixelFormat, red, green, blue, alpha);
					rle--;
				}
			}
			else // raw packet
			{
				rle++; // rle is now number of of raw pixels
				x+=rle; 
				DEBUG_FATAL(x>width, ("TargaFormat: Read invalid RLE packet.\n"));
				fileInterface->read(line, int(rle)*fileBytesPerPixel);
				uint8 *lineIter=line;
				while (rle)
				{
					index = (fileBytesPerPixel==1) ? unsigned(*lineIter) : unsigned(*(unsigned short *)lineIter);
					lookup = cmap + index*bytesPerColorEntry;
					switch (bytesPerColorEntry)
					{
					case 1:
					{
						red  =lookup[0];
						green=lookup[0];
						blue =lookup[0];
						alpha=255;
					} break;
					case 3:
					{
						red  =lookup[0];
						green=lookup[1];
						blue =lookup[2];
						alpha=255;
					} break;
					case 4:
					{
						red  =lookup[0];
						green=lookup[1];
						blue =lookup[2];
						alpha=lookup[3];
					} break;
					}
					Image::setPixel(destIter, destPixelFormat, red, green, blue, alpha);
					lineIter+=fileBytesPerPixel;
					rle--;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void TargaFormatNamespace::_readRleTrueColor(
	uint8 *destData,
	Image::PixelFormat destPixelFormat,
	AbstractFile *fileInterface,
	int width,
	int yStart, 
	int yStop, 
	int yStep, 
	int yStride,
	int fileBytesPerPixel
)
{
	uint8 red=0, green=0, blue=0, alpha=0, p[4];
	const int lineSize = width*fileBytesPerPixel;
	uint8 *line = (uint8 *)alloca(lineSize);
	uint8 rle;

	for (int y=yStart;y!=yStop;y+=yStep)
	{
		uint8 *destIter = destData + y*yStride;
		int x=0;
		while (x<width)
		{
	      fileInterface->read(&rle, 1);
			if (rle&0x80) // run length packet
			{
				rle-=127;
				x+=rle;
				DEBUG_FATAL(x>width, ("TargaFormat: Read invalid RLE packet.\n"));

				fileInterface->read(p, fileBytesPerPixel);

				switch (fileBytesPerPixel)
				{
				case 1:
				{
					red  =p[0];
					green=p[0];
					blue =p[0];
					alpha=255;
				} break;
				case 3:
				{
					red  =p[2];
					green=p[1];
					blue =p[0];
					alpha=255;
				} break;
				case 4:
				{
					red  =p[2];
					green=p[1];
					blue =p[0];
					alpha=p[3];
				} break;
				}

				while (rle)
				{
					Image::setPixel(destIter, destPixelFormat, red, green, blue, alpha);
					rle--;
				}
			}
			else // raw packet
			{
				rle++; // rle is now number of of raw pixels
				x+=rle; 
				DEBUG_FATAL(x>width, ("TargaFormat: Read invalid RLE packet.\n"));
				fileInterface->read(line, int(rle)*fileBytesPerPixel);
				uint8 *lineIter=line;
				while (rle)
				{
					switch (fileBytesPerPixel)
					{
					case 1:
					{
						red  =lineIter[0];
						green=lineIter[0];
						blue =lineIter[0];
						alpha=255;
					} break;
					case 3:
					{
						red  =lineIter[2];
						green=lineIter[1];
						blue =lineIter[0];
						alpha=255;
					} break;
					case 4:
					{
						red  =lineIter[2];
						green=lineIter[1];
						blue =lineIter[0];
						alpha=lineIter[3];
					} break;
					}
					Image::setPixel(destIter, destPixelFormat, red, green, blue, alpha);

					lineIter+=fileBytesPerPixel;
					rle--;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

bool TargaFormat::saveImage(const Image &image, const char *filename)
{
	FILE *f;

	const int width = image.getWidth();
	const int height = image.getHeight();
	const int sourceBytesPerPixel = image.getBytesPerPixel();
	const Image::PixelFormat pixelFormat = image.getPixelFormat();

	Image::PixelFormat diskPixelFormat;
	int bitDepth;
	switch (pixelFormat)
	{
	case Image::PF_bgra_8888:
		diskPixelFormat = Image::PF_bgra_8888;
		bitDepth = 32;
		break;

	case Image::PF_bgr_888:
		diskPixelFormat = Image::PF_bgr_888;
		bitDepth = 24;
		break;

	case Image::PF_abgr_8888:
		diskPixelFormat = Image::PF_bgra_8888;
		bitDepth = 32;
		break;

	case Image::PF_rgba_8888:
		diskPixelFormat = Image::PF_bgra_8888;
		bitDepth = 32;
		break;

	case Image::PF_rgb_888:
		diskPixelFormat = Image::PF_bgr_888;
		bitDepth = 24;
		break;

	case Image::PF_argb_8888:
		diskPixelFormat = Image::PF_bgra_8888;
		bitDepth = 32;
		break;

	//case Image::PF_w_8: //8bit greyscale bitmap
	//	break;

	default:
		DEBUG_WARNING(true, ("Attempt to save un-supported Targa file format.\n"));
		return false;
	}

	//---------------------------------------------------
	const uint8 *pixels = image.lockReadOnly(true);
	if (!pixels)
	{
		DEBUG_WARNING(true, ("Could not obtain pixel pointer when writing Targa file %s.\n", filename));
		return false;
	}
	Image::UnlockGuard unlockImage(&image);
	//---------------------------------------------------

	f = fopen(filename, "wb");
	if (!f)
	{
		DEBUG_WARNING(true, ("Unable to open Targa destination file %s.\n", filename));
		return false;
	}

	//------------------------------------------
	//-- Fill header
	TargaHeader header;
   header.m_idLength        = 0;   // length of optional identification sequence
   header.m_colorMapType    = 0;   // indicates whether a palette is present
   header.m_imageType       = 2;   // image data type (e.g., uncompressed RGB)
   header.m_colorMapStart   = 0;   // first palette index, if present
   header.m_colorMapLength  = 0;   // number of palette entries, if present
   header.m_colorMapDepth   = 0;   // number of bits per palette entry
   header.m_xOriginOffset   = 0;   // horiz pixel coordinate of lower left of image
   header.m_yOriginOffset   = 0;   // vert pixel coordinate of lower left of image
   header.m_width           = static_cast<unsigned short>(width);    // image width in pixels
   header.m_height          = static_cast<unsigned short>(height);   // image height in pixels
   header.m_pixelDepth      = static_cast<unsigned char>(bitDepth);  // image color depth (bits per pixel)
   header.m_imageDescriptor = 0;   // image attribute flags (lower-left origin)
	//------------------------------------------

	int numWritten;

	//------------------------------------------
	//-- Write header (fields 1 thru 5 of spec).
	numWritten = fwrite(&header, sizeof(header), 1, f);
   if (numWritten != 1)
   {
	    fclose(f);
		DEBUG_WARNING(true, ("Targa header not written successfully.\n"));
      return false;
   }
	//------------------------------------------

	//------------------------------------------
	// Image ID (field 6)
	//------------------------------------------
	// Color Map Data (field 7)
	//------------------------------------------

	//------------------------------------------
	// Image Data (field 8)
	const int sourceYStride = width * sourceBytesPerPixel;
	const byte *piter = pixels + (height-1)*width*sourceBytesPerPixel;
	if (pixelFormat!=diskPixelFormat)
	{
		const int destBytesPerPixel = bitDepth / 8;
		const int destYStride = width*destBytesPerPixel;
		byte *destPixelsLine = (byte *)alloca(destYStride);

		while (piter>=pixels)
		{
			byte *diter = destPixelsLine;
			byte *diterStop = destPixelsLine + destYStride;

			while (diter!=diterStop)
			{
				Image::copyPixel(diter, diskPixelFormat, piter, pixelFormat);
			}
			numWritten = fwrite(destPixelsLine, destYStride, 1, f);
			if (numWritten!=1)
			{
				fclose(f);
				DEBUG_WARNING(true, ("Targa image data not written successfully for %s\n", filename));
				return false;
			}

			piter-=2*sourceYStride;
		}
	}
	else
	{
		while (piter>=pixels)
		{
			numWritten = fwrite(piter, sourceYStride, 1, f);
			if (numWritten!=1)
			{
				fclose(f);
				DEBUG_WARNING(true, ("Targa image data not written successfully for %s\n", filename));
				return false;
			}
			piter-=sourceYStride;
		}
	}
	//------------------------------------------

	//------------------------------------------
	// Developer Data (field 9)
	//------------------------------------------
	// Extension Area (field 10)
	//------------------------------------------

	//------------------------------------------
	// File footer
	TargaFooter footer;
	footer.m_extensionAreaOffset=0;
	footer.m_developerDirectoryOffset=0;
	memcpy(footer.m_signature, ms_TargaSignature, TARGA_SIGNATURE_LENGTH);
	numWritten = fwrite(&footer, sizeof(footer), 1, f);
   if (numWritten != 1)
   {
	  fclose(f);
	  DEBUG_WARNING(true, ("Targa footer not written successfully for %s\n", filename));
      return false;
   }
	//------------------------------------------

	fclose(f);
	return true;
}

// ----------------------------------------------------------------------

int TargaFormat::getCommonExtensionCount() const
{
	return 1;
}

// ----------------------------------------------------------------------

const char *TargaFormat::getCommonExtension(int index) const
{
	UNREF(index);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, 1);
	return "tga";
}

// ======================================================================
