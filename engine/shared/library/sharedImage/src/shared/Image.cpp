// ======================================================================
//
// Image.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "sharedImage/FirstSharedImage.h"
#include "sharedImage/Image.h"

#include "sharedFoundation/Crc.h"

// ======================================================================
// lint supression
// ======================================================================

//lint -e1734 // info -- had difficulty compiling template function // maybe bug in lint?

// ======================================================================
// class Image non-static member functions
// ======================================================================

Image::Image()
:	m_width(0),
	m_height(0),
	m_bitsPerPixel(0),
	m_bytesPerPixel(0),
	m_stride(0),
	m_redMask(0),
	m_greenMask(0),
	m_blueMask(0),
	m_alphaMask(0),
	m_ownBuffer(true),
	m_buffer(0),
	m_bufferSize(0),
	m_readOnly(false),
	m_isLocked(false),
	m_formatDirty(true),
	m_format(PF_nonStandard)
{
}

// ----------------------------------------------------------------------

Image::Image(uint8 *pixelBuffer, int pixelBufferSize, int width, int height, int bitsPerPixel, int bytesPerPixel, int strideInBytes, uint redMask, uint greenMask, uint blueMask, uint alphaMask)
:	m_width(width),
	m_height(height),
	m_bitsPerPixel(bitsPerPixel),
	m_bytesPerPixel(bytesPerPixel),
	m_stride(strideInBytes),
	m_redMask(redMask),
	m_greenMask(greenMask),
	m_blueMask(blueMask),
	m_alphaMask(alphaMask),
	m_ownBuffer(false),
	m_buffer(pixelBuffer),
	m_bufferSize(pixelBufferSize),
	m_readOnly(false),
	m_isLocked(false),
	m_formatDirty(true),
	m_format(PF_nonStandard)
{
}

// ----------------------------------------------------------------------

Image::~Image()
{
	if (m_ownBuffer)
		delete [] m_buffer;
	else
		m_buffer = 0;
}

// ----------------------------------------------------------------------

void Image::setDimensions(int width, int height, int bitsPerPixel, int bytesPerPixel)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(1, bitsPerPixel, 32);
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(1, bytesPerPixel, 4);
	DEBUG_FATAL( ((static_cast<uint>(bitsPerPixel)+7)>>3) > static_cast<uint>(bytesPerPixel), ("invalid bytesPerPixel set, %d bytes with %d  bits\n", bytesPerPixel, bitsPerPixel));
	FATAL(m_isLocked, ("image is locked"));

	//-- calculate default stride
	const int stride = bytesPerPixel * width;

	//-- set the dimensions
	setDimensions(width, height, bitsPerPixel, bytesPerPixel, stride);
}

// ----------------------------------------------------------------------

void Image::setDimensions(int width, int height, int bitsPerPixel, int bytesPerPixel, int strideInBytes)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(1, bitsPerPixel, 32);
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(1, bytesPerPixel, 4);
	DEBUG_FATAL( ((static_cast<uint>(bitsPerPixel)+7)>>3) > static_cast<uint>(bytesPerPixel), ("invalid bytesPerPixel set, %d bytes with %d  bits\n", bytesPerPixel, bitsPerPixel));
	FATAL(m_isLocked, ("image is locked"));

	//-- save image dimensions
	m_width         = width;
	m_height        = height;
	m_bitsPerPixel  = bitsPerPixel;
	m_bytesPerPixel = bytesPerPixel;
	m_stride        = strideInBytes;

	//-- calculate buffer size
	m_bufferSize = strideInBytes * height;
	if (strideInBytes < 0)
		m_bufferSize = -m_bufferSize;
	FATAL(m_bufferSize < 0, ("bad buffer size [%d], width=[%d], height=[%d], stride=[%d]", m_bufferSize, width, height, strideInBytes));
	
	//-- create the buffer
	delete [] m_buffer;
	m_buffer = new uint8[static_cast<uint>(m_bufferSize)];

	//-- format is now dirty
	m_formatDirty = true;
}

// ----------------------------------------------------------------------

void Image::setPixelInformation(uint redMask, uint greenMask, uint blueMask, uint alphaMask)
{
	FATAL(m_isLocked, ("image is locked"));

	m_redMask   = redMask;
	m_greenMask = greenMask;
	m_blueMask  = blueMask;
	m_alphaMask = alphaMask;

	//-- format info now is dirty
	m_formatDirty = true;
}

// ----------------------------------------------------------------------

uint8 *Image::lock(bool optional)
{
	//-- check for error conditions
	if (optional)
	{
		// bail out gracefully on errors
		if (m_isLocked)
		{
			REPORT_LOG(true, ("tried to lock() already locked image\n"));
			return m_buffer;
		}

		if (m_readOnly)
		{
			REPORT_LOG(true, ("tried to lock() read-only image\n"));
			return 0;
		}
	}
	else
	{
		FATAL(m_isLocked, ("tried to lock() already locked image\n"));
		FATAL(m_readOnly, ("tried to lock() read-only image\n"));
	}

	//-- lock it
	m_isLocked = true;
	return m_buffer;
}

// ----------------------------------------------------------------------

const uint8 *Image::lockReadOnly(bool optional) const
{
	//-- check for error conditions
	if (optional)
	{
		// bail out gracefully on errors
		if (m_isLocked)
		{
			REPORT_LOG(true, ("tried to lockReadOnly() already locked image\n"));
			return m_buffer;
		}
	}
	else
	{
		FATAL(m_isLocked, ("tried to lockReadOnly() already locked image\n"));
	}

	//-- lock it
	m_isLocked = true;
	return m_buffer;
}

// ----------------------------------------------------------------------

void Image::unlock() const
{
	FATAL(!m_isLocked, ("image not locked"));
	
	m_isLocked = false;
	return;
}

// ----------------------------------------------------------------------

uint32 Image::calculateCrc() const
{
	FATAL(!m_buffer, ("no buffer"));
	return Crc::calculate(m_buffer, m_bufferSize);
}

// ----------------------------------------------------------------------

Image::PixelFormat Image::calculatePixelFormat() const
{
	const int redShiftCount   = GetFirstBitSet(m_redMask);
	const int greenShiftCount = GetFirstBitSet(m_greenMask);
	const int blueShiftCount  = GetFirstBitSet(m_blueMask);
	const int alphaShiftCount = GetFirstBitSet(m_alphaMask);

	const int redBitCount     = GetBitCount(m_redMask);
	const int greenBitCount   = GetBitCount(m_greenMask);
	const int blueBitCount    = GetBitCount(m_blueMask);
	const int alphaBitCount   = GetBitCount(m_alphaMask);

	if ((blueShiftCount < greenShiftCount) && (greenShiftCount < redShiftCount))
	{
		// bgr layout
		if (!m_alphaMask)
		{
			if ((redBitCount == 8) && (greenBitCount == 8) && (blueBitCount == 8))
				return PF_bgr_888;
			else
				return PF_nonStandard;
		}
		else
		{
			if (alphaShiftCount > redShiftCount)
			{
				if ((redBitCount == 8) && (greenBitCount == 8) && (blueBitCount == 8) && (alphaBitCount == 8))
					return PF_bgra_8888;
				else
					return PF_nonStandard;
			}
			else if (alphaShiftCount < blueShiftCount)
			{
				if ((redBitCount == 8) && (greenBitCount == 8) && (blueBitCount == 8) && (alphaBitCount == 8))
					return PF_abgr_8888;
				else
					return PF_nonStandard;
			}
			else
			{
				// don't know where this alpha is.  non-standard
				return PF_nonStandard;
			}
		}		
	}
	else if ((redShiftCount < greenShiftCount) && (greenShiftCount < blueShiftCount))
	{
		// rgb layout
		if (!m_alphaMask)
		{
			if ((redBitCount == 8) && (greenBitCount == 8) && (blueBitCount == 8))
				return PF_rgb_888;
			else
				return PF_nonStandard;
		}
		else
		{
			if (alphaShiftCount > blueShiftCount)
			{
				if ((redBitCount == 8) && (greenBitCount == 8) && (blueBitCount == 8) && (alphaBitCount == 8))
					return PF_rgba_8888;
				else
					return PF_nonStandard;
			}
			else if (alphaShiftCount < redShiftCount)
			{
				if ((redBitCount == 8) && (greenBitCount == 8) && (blueBitCount == 8) && (alphaBitCount == 8))
					return PF_argb_8888;
				else
					return PF_nonStandard;
			}
			else
			{
				// don't know where this alpha is.  non-standard
				return PF_nonStandard;
			}
		}		
	}
	else if(m_redMask == 0 && m_greenMask == 0 && m_blueMask == 0 && m_alphaMask == 0 && m_bitsPerPixel == 8)
	{
		return PF_w_8; // 8bit greyscale bitmap
	}
	else
	{
		return PF_nonStandard;
	}
}

// ======================================================================
