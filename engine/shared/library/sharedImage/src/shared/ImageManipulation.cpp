// ======================================================================
//
// ImageManipulation.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedImage/FirstSharedImage.h"
#include "sharedImage/ImageManipulation.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedImage/Image.h"

// ======================================================================

bool                                      ImageManipulation::ms_installed;

ImageManipulation::NextMipmapFunction     ImageManipulation::ms_nextMipmapFunction;
ImageManipulation::ConvertFormatFunction  ImageManipulation::ms_convertFormatFunction;
ImageManipulation::CopyFunction           ImageManipulation::ms_copyIgnoreAlphaFunction;
ImageManipulation::CopyFunction           ImageManipulation::ms_copyRespectAlphaFunction;
ImageManipulation::BlendFunction          ImageManipulation::ms_blendFunction;
ImageManipulation::BlendTwoFunction       ImageManipulation::ms_blendTwoFunction;

// ======================================================================

namespace ImageManipulationNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<typename PixelTraits>
	struct DefaultNextMipmapFunction
	{
		static void nextMipmapFunction(const Image &sourceImage, Image &destImage);
		static void nextMipmapFunction_EqualWidth(const Image &sourceImage, Image &destImage);
		static void nextMipmapFunction_EqualHeight(const Image &sourceImage, Image &destImage);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<const int redIndex, const int greenIndex, const int blueIndex>
	struct PixelTraitsThreeByOne
	{
		static int   getPixelStride();
		static void  copyWithPowerOfTwoDivide(const uint8 *sourcePixel, uint8 *destPixel, uint8 powerOfTwoDivide);
		static void  addWithPowerOfTwoDivide(const uint8 *sourcePixel, uint8 *destPixel, uint8 powerOfTwoDivide);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template<const int redIndex, const int greenIndex, const int blueIndex, const int alphaIndex>
	struct PixelTraitsFourByOne
	{
		static int   getPixelStride();
		static void  copyWithPowerOfTwoDivide(const uint8 *sourcePixel, uint8 *destPixel, uint8 powerOfTwoDivide);
		static void  addWithPowerOfTwoDivide(const uint8 *sourcePixel, uint8 *destPixel, uint8 powerOfTwoDivide);
	};
}

using namespace ImageManipulationNamespace;

// ======================================================================

template<typename PixelTraits>
void ImageManipulationNamespace::DefaultNextMipmapFunction<PixelTraits>::nextMipmapFunction(const Image &sourceImage, Image &destImage)
{
	const uint8 *const  sourceBuffer = sourceImage.lockReadOnly();
	const uint8        *sourceLine   = sourceBuffer;
	const int           sourceStride = sourceImage.getStride();

	const int           destHeight   = destImage.getHeight();
	const int           destWidth    = destImage.getWidth();
	uint8 *const        destBuffer   = destImage.lock();
	uint8              *destLine     = destBuffer;
	const int           destStride   = destImage.getStride();

	// The strategy we follow in this algorithm attempts to optimize cache coherency.
	// Each dest pixel is written with a simple 2x2 box filter over the source pixels.
	// Each source pixel weights at 25%.  So that we always read linearly from the
	// source data, we write each dest line in two passes.  The first pass writes in
	// the pixel contribution to the pixel from the first source line.  The
	// second pass writes in the contribution from the second source line.

	for (int destY = 0; destY < destHeight; ++destY)
	{
		//-- read first source line, applying 25% of each source pixel into dest pixel
		const uint8 *sourcePixel = sourceLine;
		uint8       *destPixel   = destLine;

		{
			for (int destX = 0; destX < destWidth; ++destX)
			{
				//-- handle source upper left (first pixel gets copied in)
				PixelTraits::copyWithPowerOfTwoDivide(sourcePixel, destPixel, 2);
				sourcePixel += PixelTraits::getPixelStride();

				//-- handle source upper right
				PixelTraits::addWithPowerOfTwoDivide(sourcePixel, destPixel, 2);
				sourcePixel += PixelTraits::getPixelStride();

				//-- move on to next dest pixel
				destPixel   += PixelTraits::getPixelStride();
			}
		}

		//-- read second source line, applying 25% of each source pixel into dest pixel
		sourceLine += sourceStride;

		sourcePixel = sourceLine;
		destPixel   = destLine;

		{
			for (int destX = 0; destX < destWidth; ++destX)
			{
				//-- handle source lower left
				PixelTraits::addWithPowerOfTwoDivide(sourcePixel, destPixel, 2);
				sourcePixel += PixelTraits::getPixelStride();

				//-- handle source lower right
				PixelTraits::addWithPowerOfTwoDivide(sourcePixel, destPixel, 2);
				sourcePixel += PixelTraits::getPixelStride();

				//-- move on to next dest pixel
				destPixel   += PixelTraits::getPixelStride();
			}
		}

		//-- increment source and dest line pointers
		destLine   += destStride;
		sourceLine += sourceStride;
	}

	//-- cleanup
	destImage.unlock();
	sourceImage.unlock();
}

// ----------------------------------------------------------------------
/**
 * This function generates the next smaller mipmap in the Y direction but
 * preserves the image in the x direction.
 *
 * This function would be used when we are mipmapping a 1x8 down to a 1x4.
 * The X contribution to color is 50% for the current pixel.
 */

template<typename PixelTraits>
void ImageManipulationNamespace::DefaultNextMipmapFunction<PixelTraits>::nextMipmapFunction_EqualWidth(const Image &sourceImage, Image &destImage)
{
	DEBUG_FATAL(sourceImage.getWidth() != destImage.getWidth(), ("This function only makes sense when source width [%d] is the same as dest width [%d].", sourceImage.getWidth(), destImage.getWidth()));

	const uint8 *const  sourceBuffer = sourceImage.lockReadOnly();
	const uint8        *sourceLine   = sourceBuffer;
	const int           sourceStride = sourceImage.getStride();

	const int           destHeight   = destImage.getHeight();
	const int           destWidth    = destImage.getWidth();
	uint8 *const        destBuffer   = destImage.lock();
	uint8              *destLine     = destBuffer;
	const int           destStride   = destImage.getStride();

	// The strategy we follow in this algorithm attempts to optimize cache coherency.
	// Each dest pixel is written with a simple 1x2 box filter over the source pixels.
	// Each source pixel weights at 50%.  So that we always read linearly from the
	// source data, we write each dest line in two passes.  The first pass writes in
	// the pixel contribution to the pixel from the first source line.  The
	// second pass writes in the contribution from the second source line.

	for (int destY = 0; destY < destHeight; ++destY)
	{
		//-- read first source line, applying 50% of each source pixel into dest pixel
		const uint8 *sourcePixel = sourceLine;
		uint8       *destPixel   = destLine;

		{
			for (int destX = 0; destX < destWidth; ++destX)
			{
				//-- handle upper pixel (first pixel gets copied in)
				PixelTraits::copyWithPowerOfTwoDivide(sourcePixel, destPixel, 1);

				//-- move on to next pixel
				sourcePixel += PixelTraits::getPixelStride();
				destPixel   += PixelTraits::getPixelStride();
			}
		}

		//-- read second source line, applying 50% of each source pixel into dest pixel
		sourceLine += sourceStride;

		sourcePixel = sourceLine;
		destPixel   = destLine;

		{
			for (int destX = 0; destX < destWidth; ++destX)
			{
				//-- handle source lower pixel
				PixelTraits::addWithPowerOfTwoDivide(sourcePixel, destPixel, 1);

				//-- move on to next dest pixel
				sourcePixel += PixelTraits::getPixelStride();
				destPixel   += PixelTraits::getPixelStride();
			}
		}

		//-- increment source and dest line pointers
		destLine   += destStride;
		sourceLine += sourceStride;
	}

	//-- cleanup
	destImage.unlock();
	sourceImage.unlock();
}

// ----------------------------------------------------------------------
/**
 * This function generates the next smaller mipmap in the X direction but
 * preserves the image in the Y direction.
 *
 * This function would be used when we are mipmapping an 8x1 down to a 4x1.
 * The Y contribution to color is 50% for the current pixel.
 */


template<typename PixelTraits>
void ImageManipulationNamespace::DefaultNextMipmapFunction<PixelTraits>::nextMipmapFunction_EqualHeight(const Image &sourceImage, Image &destImage)
{
	DEBUG_FATAL(sourceImage.getHeight() != destImage.getHeight(), ("This function only makes sense when source height [%d] is the same as dest height [%d].", sourceImage.getHeight(), destImage.getHeight()));

	const uint8 *const  sourceBuffer = sourceImage.lockReadOnly();
	const uint8        *sourceLine   = sourceBuffer;
	const int           sourceStride = sourceImage.getStride();

	const int           destHeight   = destImage.getHeight();
	const int           destWidth    = destImage.getWidth();
	uint8 *const        destBuffer   = destImage.lock();
	uint8              *destLine     = destBuffer;
	const int           destStride   = destImage.getStride();

	// The strategy we follow in this algorithm attempts to optimize cache coherency.
	// Each dest pixel is written with a simple 2x1 box filter over the source pixels.
	// Each source pixel weights at 50%.

	for (int destY = 0; destY < destHeight; ++destY)
	{
		//-- read first source line, applying 50% of each source pixel into dest pixel
		const uint8 *sourcePixel = sourceLine;
		uint8       *destPixel   = destLine;

		{
			for (int destX = 0; destX < destWidth; ++destX)
			{
				//-- handle source left (first pixel gets copied in)
				PixelTraits::copyWithPowerOfTwoDivide(sourcePixel, destPixel, 1);
				sourcePixel += PixelTraits::getPixelStride();

				//-- handle source right
				PixelTraits::addWithPowerOfTwoDivide(sourcePixel, destPixel, 1);
				sourcePixel += PixelTraits::getPixelStride();

				//-- move on to next dest pixel
				destPixel   += PixelTraits::getPixelStride();
			}
		}

		//-- increment source and dest line pointers
		destLine   += destStride;
		sourceLine += sourceStride;
	}

	//-- cleanup
	destImage.unlock();
	sourceImage.unlock();
}

// ======================================================================
// template class PixelTraitsThreeByOne
// ======================================================================

template<const int redIndex, const int greenIndex, const int blueIndex>
inline int ImageManipulationNamespace::PixelTraitsThreeByOne<redIndex, greenIndex, blueIndex>::getPixelStride()
{
	return 3;
}

// ----------------------------------------------------------------------

template<const int redIndex, const int greenIndex, const int blueIndex>
inline void ImageManipulationNamespace::PixelTraitsThreeByOne<redIndex, greenIndex, blueIndex>::copyWithPowerOfTwoDivide(const uint8 *sourcePixel, uint8 *destPixel, uint8 powerOfTwoDivide)
{
	destPixel[0] = static_cast<uint8>(sourcePixel[0] >> powerOfTwoDivide);
	destPixel[1] = static_cast<uint8>(sourcePixel[1] >> powerOfTwoDivide);
	destPixel[2] = static_cast<uint8>(sourcePixel[2] >> powerOfTwoDivide);
}

// ----------------------------------------------------------------------

template<const int redIndex, const int greenIndex, const int blueIndex>
inline void ImageManipulationNamespace::PixelTraitsThreeByOne<redIndex, greenIndex, blueIndex>::addWithPowerOfTwoDivide(const uint8 *sourcePixel, uint8 *destPixel, uint8 powerOfTwoDivide)
{
	destPixel[0] = static_cast<uint8>(destPixel[0] + static_cast<uint8>(sourcePixel[0] >> powerOfTwoDivide));
	destPixel[1] = static_cast<uint8>(destPixel[1] + static_cast<uint8>(sourcePixel[1] >> powerOfTwoDivide));
	destPixel[2] = static_cast<uint8>(destPixel[2] + static_cast<uint8>(sourcePixel[2] >> powerOfTwoDivide));
}

// ======================================================================
// template class PixelTraitsFourByOne
// ======================================================================

template<const int redIndex, const int greenIndex, const int blueIndex, const int alphaIndex>
inline int ImageManipulationNamespace::PixelTraitsFourByOne<redIndex, greenIndex, blueIndex, alphaIndex>::getPixelStride()
{
	return 4;
}

// ----------------------------------------------------------------------

template<const int redIndex, const int greenIndex, const int blueIndex, const int alphaIndex>
inline void ImageManipulationNamespace::PixelTraitsFourByOne<redIndex, greenIndex, blueIndex, alphaIndex>::copyWithPowerOfTwoDivide(const uint8 *sourcePixel, uint8 *destPixel, uint8 powerOfTwoDivide)
{
	destPixel[0] = static_cast<uint8>(sourcePixel[0] >> powerOfTwoDivide);
	destPixel[1] = static_cast<uint8>(sourcePixel[1] >> powerOfTwoDivide);
	destPixel[2] = static_cast<uint8>(sourcePixel[2] >> powerOfTwoDivide);
	destPixel[3] = static_cast<uint8>(sourcePixel[3] >> powerOfTwoDivide);
}

// ----------------------------------------------------------------------

template<const int redIndex, const int greenIndex, const int blueIndex, const int alphaIndex>
inline void ImageManipulationNamespace::PixelTraitsFourByOne<redIndex, greenIndex, blueIndex, alphaIndex>::addWithPowerOfTwoDivide(const uint8 *sourcePixel, uint8 *destPixel, uint8 powerOfTwoDivide)
{
	destPixel[0] = static_cast<uint8>(destPixel[0] + static_cast<uint8>(sourcePixel[0] >> powerOfTwoDivide));
	destPixel[1] = static_cast<uint8>(destPixel[1] + static_cast<uint8>(sourcePixel[1] >> powerOfTwoDivide));
	destPixel[2] = static_cast<uint8>(destPixel[2] + static_cast<uint8>(sourcePixel[2] >> powerOfTwoDivide));
	destPixel[3] = static_cast<uint8>(destPixel[3] + static_cast<uint8>(sourcePixel[3] >> powerOfTwoDivide));
}

// ======================================================================
// struct ImageManipulation::InstallData
// ======================================================================

ImageManipulation::InstallData::InstallData()
:	m_nextMipmapFunction(0),
	m_convertFormatFunction(0),
	m_copyIgnoreAlphaFunction(0),
	m_copyRespectAlphaFunction(0),
	m_blendFunction(0),
	m_blendTwoFunction(0)
{
}

// ======================================================================
// class ImageManipulation
// ======================================================================

void ImageManipulation::install(const InstallData &installData)
{
	DEBUG_FATAL(ms_installed, ("ImageManipulation already installed"));

	ms_installed = true;
	ExitChain::add(remove, "ImageManipulation");

	if (installData.m_nextMipmapFunction)
		ms_nextMipmapFunction = installData.m_nextMipmapFunction;
	else
		ms_nextMipmapFunction = defaultNextMipmapFunction;

	//-- make sure we've got valid function pointers for all
	NOT_NULL(ms_nextMipmapFunction);
}

// ----------------------------------------------------------------------

void ImageManipulation::remove()
{
	DEBUG_FATAL(!ms_installed, ("ImageManipulation not installed"));

	ms_nextMipmapFunction       = 0;
	ms_convertFormatFunction    = 0;
	ms_copyIgnoreAlphaFunction  = 0;
	ms_copyRespectAlphaFunction = 0;
	ms_blendFunction            = 0;
	ms_blendTwoFunction         = 0;
}

// ----------------------------------------------------------------------

void ImageManipulation::defaultNextMipmapFunction(const Image &sourceImage, Image &destImage)
{
	const Image::PixelFormat pixelFormat = sourceImage.getPixelFormat();

	DEBUG_FATAL(pixelFormat != destImage.getPixelFormat(), ("source and dest image have different pixel layout"));
	DEBUG_FATAL(pixelFormat == Image::PF_nonStandard, ("cannot handle non-standard pixel layouts"));

	DEBUG_FATAL(!IsPowerOfTwo(sourceImage.getWidth()), ("source width not power of two %d", sourceImage.getWidth()));
	DEBUG_FATAL(!IsPowerOfTwo(sourceImage.getHeight()), ("source height not power of two %d", sourceImage.getHeight()));
	
	const int destWidth    = destImage.getWidth();
	const int destHeight   = destImage.getHeight();
	const int sourceWidth  = sourceImage.getWidth();
	const int sourceHeight = sourceImage.getHeight();

	DEBUG_FATAL(destWidth < 1, ("destImage width invalid [%d].", destWidth));
	DEBUG_FATAL(sourceWidth < 1, ("sourceImage width invalid [%d].", sourceWidth));
	DEBUG_FATAL(destHeight < 1, ("destImage height invalid [%d].", destHeight));
	DEBUG_FATAL(sourceHeight < 1, ("sourceImage height invalid [%d].", sourceHeight));

	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(sourceWidth / 2, destWidth, sourceWidth);
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(sourceHeight / 2, destHeight, sourceHeight);

	//-- select the proper templated function
	NextMipmapFunction  workerFunction = 0;

	if ((destWidth == sourceWidth / 2) && (destHeight == sourceHeight / 2))
	{
		// Mipmap down in both x and y direction.
		switch (pixelFormat)
		{
			case Image::PF_bgr_888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsThreeByOne<2, 1, 0> >::nextMipmapFunction;
				break;

			case Image::PF_rgb_888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsThreeByOne<0, 1, 2> >::nextMipmapFunction;
				break;

			case Image::PF_bgra_8888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsFourByOne<2, 1, 0, 3> >::nextMipmapFunction;
				break;

			case Image::PF_abgr_8888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsFourByOne<3, 2, 1, 0> >::nextMipmapFunction;
				break;

			case Image::PF_rgba_8888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsFourByOne<0, 1, 2, 3> >::nextMipmapFunction;
				break;

			case Image::PF_argb_8888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsFourByOne<1, 2, 3, 0> >::nextMipmapFunction;
				break;

			case Image::PF_nonStandard:
			default:
				FATAL(true, ("unsupported format for conversion"));
		}
	}
	else if ((destWidth == sourceWidth) && (destHeight == sourceHeight / 2))
	{
		// Mipmap down in y, preserve x direction.
		switch (pixelFormat)
		{
			case Image::PF_bgr_888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsThreeByOne<2, 1, 0> >::nextMipmapFunction_EqualWidth;
				break;

			case Image::PF_rgb_888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsThreeByOne<0, 1, 2> >::nextMipmapFunction_EqualWidth;
				break;

			case Image::PF_bgra_8888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsFourByOne<2, 1, 0, 3> >::nextMipmapFunction_EqualWidth;
				break;

			case Image::PF_abgr_8888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsFourByOne<3, 2, 1, 0> >::nextMipmapFunction_EqualWidth;
				break;

			case Image::PF_rgba_8888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsFourByOne<0, 1, 2, 3> >::nextMipmapFunction_EqualWidth;
				break;

			case Image::PF_argb_8888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsFourByOne<1, 2, 3, 0> >::nextMipmapFunction_EqualWidth;
				break;

			case Image::PF_nonStandard:
			default:
				FATAL(true, ("unsupported format for conversion"));
		}
	}
	else if ((destWidth == sourceWidth / 2) && (destHeight == sourceHeight))
	{
		// Mipmap down in y, preserve x direction.
		switch (pixelFormat)
		{
			case Image::PF_bgr_888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsThreeByOne<2, 1, 0> >::nextMipmapFunction_EqualHeight;
				break;

			case Image::PF_rgb_888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsThreeByOne<0, 1, 2> >::nextMipmapFunction_EqualHeight;
				break;

			case Image::PF_bgra_8888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsFourByOne<2, 1, 0, 3> >::nextMipmapFunction_EqualHeight;
				break;

			case Image::PF_abgr_8888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsFourByOne<3, 2, 1, 0> >::nextMipmapFunction_EqualHeight;
				break;

			case Image::PF_rgba_8888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsFourByOne<0, 1, 2, 3> >::nextMipmapFunction_EqualHeight;
				break;

			case Image::PF_argb_8888:
				workerFunction = DefaultNextMipmapFunction<PixelTraitsFourByOne<1, 2, 3, 0> >::nextMipmapFunction_EqualHeight;
				break;

			case Image::PF_nonStandard:
			default:
				FATAL(true, ("unsupported format for conversion"));
		}
	}
	else
		FATAL(true, ("Unexpected error: mipmap generation failing [source w=%d,h=%d], [dest w=%d,h=%d].", sourceWidth, sourceHeight, destWidth, destHeight));

	NOT_NULL(workerFunction);

	//-- call the function
	(*workerFunction)(sourceImage, destImage);
}

// ======================================================================
