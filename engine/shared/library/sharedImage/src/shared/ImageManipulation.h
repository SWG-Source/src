// ======================================================================
//
// ImageManipulation.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ImageManipulation_H
#define INCLUDED_ImageManipulation_H

// ======================================================================

class Image;

// ======================================================================

/**
 * This class encapsulates functionality to manipulate pre-existing Images.
 *
 * This class provides an interface for manipulating images.  This includes
 * generating mipmap levels and converting from one format to another.
 * This functionality was not included as part of class Image because all
 * these functions deal with multiple images.  Perhaps more importantly,
 * each one of these functions can be implemented in multiple ways on a
 * single platform dependent on the processor (e.g. Intel: SSE or CPU ALU).
 */

class ImageManipulation
{
public:

	typedef void (*NextMipmapFunction)(const Image &sourceImage, Image &destImage);
	typedef void (*ConvertFormatFunction)(const Image &sourceImage, Image &destImage);
	typedef void (*CopyFunction)(const Image &sourceImage, int sourceX, int sourceY, int width, int height, Image &destImage, int destX, int destY);
	typedef void (*BlendFunction)(float sourceBlendFactor, const Image &sourceImage, int sourceX, int sourceY, int width, int height, Image &destImage, int destX, int destY);
	typedef void (*BlendTwoFunction)(float firstSourceBlendFactor, float resultToDestBlendFactor, const Image &sourceImage1, const Image &sourceImage2, int sourceX, int sourceY, int width, int height, Image &destImage, int destX, int destY);

	struct InstallData
	{
	public:
	
		InstallData();

	public:

		NextMipmapFunction     m_nextMipmapFunction;
		ConvertFormatFunction  m_convertFormatFunction;
		CopyFunction           m_copyIgnoreAlphaFunction;
		CopyFunction           m_copyRespectAlphaFunction;
		BlendFunction          m_blendFunction;
		BlendTwoFunction       m_blendTwoFunction;
	};

public:

	static void  install(const InstallData &installData);

	static void  generateNextSmallerMipmap(const Image &sourceImage, Image &destImage);
private:

	static void  remove();

	static void  defaultNextMipmapFunction(const Image &sourceImage, Image &destImage);

private:

	static bool                   ms_installed;

	static NextMipmapFunction     ms_nextMipmapFunction;
	static ConvertFormatFunction  ms_convertFormatFunction;
	static CopyFunction           ms_copyIgnoreAlphaFunction;
	static CopyFunction           ms_copyRespectAlphaFunction;
	static BlendFunction          ms_blendFunction;
	static BlendTwoFunction       ms_blendTwoFunction;

};

// ======================================================================
/**
 * Generate the next smaller mipmap level from sourceImage and place in
 * destImage.
 *
 * sourceImage and destImage must be the same pixel format.
 *
 * sourceImage width and height must be a power of two, with no length
 * less than 1.
 *
 * for now, assume we always preserve texture length aspect ratio.
 * thus, we do not support generating the next mipmap level down from
 * an 8x1 texture since we cannot have a length less than one.
 *
 * sourceImage and destImage are unlocked upon entry.  they will be
 * unlocked upon exit.
 */

inline void ImageManipulation::generateNextSmallerMipmap(const Image &sourceImage, Image &destImage)
{
	DEBUG_FATAL(!ms_installed, ("ImageManipulation not installed"));

	(*ms_nextMipmapFunction)(sourceImage, destImage);
}

// ======================================================================

#endif
