// ======================================================================
//
// Image.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_Image_H
#define INCLUDED_Image_H

// ======================================================================

class Image
{
public:

	/**
	 * Memory layout of pixels starting with component at most significant bit of first byte
	 * of a pixel in memory.
	 *
	 * For x86, this is the highest bit of the least significant byte.  For big-endian
	 * architecture, this is the highest bit of the most-significant byte of a pixel.
	 */
	enum PixelFormat
	{
		PF_bgra_8888,
		PF_bgr_888,
		PF_abgr_8888,
		PF_rgba_8888,
		PF_rgb_888,
		PF_argb_8888,
		PF_w_8,        //8bit greyscale bitmap
		PF_nonStandard
	};

	static inline void setPixel(uint8 *&o_pixel, PixelFormat pixelFormat, uint8 red, uint8 green, uint8 blue, uint8 alpha=255);
	static inline void getPixel(uint8 &red, uint8 &green, uint8 &blue, uint8 &alpha, const uint8 *&i_pixel, PixelFormat pixelFormat);
	static inline void copyPixel(uint8 *&o_pixel, PixelFormat destPixelFormat, const uint8 *&i_pixel, PixelFormat sourcePixelFormat);

public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	class UnlockGuard
	{
	public:
		UnlockGuard(const Image *x) : image(x) {}
		~UnlockGuard()                         { if (image) image->unlock(); }
	private:
		UnlockGuard &operator=(const UnlockGuard &);

		const Image *const image;
	};
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Image();
	Image(uint8 *pixelBuffer, int pixelBufferSize, int width, int height, int bitsPerPixel, int bytesPerPixel, int strideInBytes, uint redMask, uint greenMask, uint blueMask, uint alphaMask);

	~Image();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// image construction interface
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void   setDimensions(int width, int height, int bitsPerPixel, int bytesPerPixel);
	void   setDimensions(int width, int height, int bitsPerPixel, int bytesPerPixel, int strideInBytes);

	void   setPixelInformation(uint redMask, uint greenMask, uint blueMask, uint alphaMask);

	uint8 *lock(bool optional = false);

	void   setReadOnly();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// image query/read interface
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	int          getWidth() const;
	int          getHeight() const;
	int          getBitsPerPixel() const;
	int          getBytesPerPixel() const;
	int          getStride() const;
	int          getBufferSize() const;

	uint         getRedMask() const;
	uint         getGreenMask() const;
	uint         getBlueMask() const;
	uint         getAlphaMask() const;

	const uint8 *lockReadOnly(bool optional = false) const;

	uint32       calculateCrc() const;
	PixelFormat  getPixelFormat() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// common interface
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void         unlock() const;

private:

	PixelFormat  calculatePixelFormat() const;

private:

	int                  m_width;
	int                  m_height;
	int                  m_bitsPerPixel;
	int                  m_bytesPerPixel;
	int                  m_stride;

	uint                 m_redMask;
	uint                 m_greenMask;
	uint                 m_blueMask;
	uint                 m_alphaMask;

	bool                 m_ownBuffer;
	uint8               *m_buffer;
	int                  m_bufferSize;

	bool                 m_readOnly;
	mutable bool         m_isLocked;

	mutable bool         m_formatDirty;
	mutable PixelFormat  m_format;
};

// ======================================================================

inline void Image::setReadOnly()
{
	m_readOnly = true;
}

// ----------------------------------------------------------------------

inline int Image::getWidth() const
{
	return m_width;
}

// ----------------------------------------------------------------------

inline int Image::getHeight() const
{
	return m_height;
}

// ----------------------------------------------------------------------

inline int Image::getBitsPerPixel() const
{
	return m_bitsPerPixel;
}

// ----------------------------------------------------------------------

inline int Image::getBytesPerPixel() const
{
	return m_bytesPerPixel;
}

// ----------------------------------------------------------------------

inline int Image::getStride() const
{
	return m_stride;
}

// ----------------------------------------------------------------------

inline int Image::getBufferSize() const
{
	return m_bufferSize;
}

// ----------------------------------------------------------------------

inline uint Image::getRedMask() const
{
	return m_redMask;
}

// ----------------------------------------------------------------------

inline uint Image::getGreenMask() const
{
	return m_greenMask;
}

// ----------------------------------------------------------------------

inline uint Image::getBlueMask() const
{
	return m_blueMask;
}

// ----------------------------------------------------------------------

inline uint Image::getAlphaMask() const
{
	return m_alphaMask;
}

// ----------------------------------------------------------------------

inline Image::PixelFormat Image::getPixelFormat() const
{
	if (m_formatDirty)
	{
		m_format      = calculatePixelFormat();
		m_formatDirty = false;
	}

	return m_format;
}

// ----------------------------------------------------------------------

inline void Image::setPixel(uint8 *&o_pixel, PixelFormat pixelFormat, uint8 red, uint8 green, uint8 blue, uint8 alpha)
{
	switch (pixelFormat)
	{
	case PF_bgra_8888:
		o_pixel[0]=blue;
		o_pixel[1]=green;
		o_pixel[2]=red;
		o_pixel[3]=alpha;
		o_pixel+=4;
		break;
	case PF_bgr_888:
		o_pixel[0]=blue;
		o_pixel[1]=green;
		o_pixel[2]=red;
		o_pixel+=3;
		break;
	case PF_abgr_8888:
		o_pixel[0]=alpha;
		o_pixel[1]=blue;
		o_pixel[2]=green;
		o_pixel[3]=red;
		o_pixel+=4;
		break;
	case PF_rgba_8888:
		o_pixel[0]=red;
		o_pixel[1]=green;
		o_pixel[2]=blue;
		o_pixel[3]=alpha;
		o_pixel+=4;
		break;
	case PF_rgb_888:
		o_pixel[0]=red;
		o_pixel[1]=green;
		o_pixel[2]=blue;
		o_pixel+=3;
		break;
	case PF_argb_8888:
		o_pixel[0]=alpha;
		o_pixel[1]=red;
		o_pixel[2]=green;
		o_pixel[3]=blue;
		o_pixel+=4;
		break;
	case PF_w_8:
		o_pixel[0]=red;
		o_pixel+=1;
		break;
	case PF_nonStandard:
		break;
	}
}

// ----------------------------------------------------------------------

inline void Image::getPixel(uint8 &red, uint8 &green, uint8 &blue, uint8 &alpha, const uint8 *&i_pixel, PixelFormat pixelFormat)
{
	switch (pixelFormat)
	{
	case PF_bgra_8888:
		blue =i_pixel[0];
		green=i_pixel[1];
		red  =i_pixel[2];
		alpha=i_pixel[3];
		i_pixel+=4;
		break;
	case PF_bgr_888:
		blue =i_pixel[0];
		green=i_pixel[1];
		red  =i_pixel[2];
		alpha=255;
		i_pixel+=3;
		break;
	case PF_abgr_8888:
		alpha=i_pixel[0];
		blue =i_pixel[1];
		green=i_pixel[2];
		red  =i_pixel[3];
		i_pixel+=4;
		break;
	case PF_rgba_8888:
		red  =i_pixel[0];
		green=i_pixel[1];
		blue =i_pixel[2];
		alpha=i_pixel[3];
		i_pixel+=4;
		break;
	case PF_rgb_888:
		red  =i_pixel[0];
		green=i_pixel[1];
		blue =i_pixel[2];
		alpha=255;
		i_pixel+=3;
		break;
	case PF_argb_8888:
		alpha=i_pixel[0];
		red  =i_pixel[1];
		green=i_pixel[2];
		blue =i_pixel[3];
		i_pixel+=4;
		break;
	case PF_w_8:
		red=blue=green=i_pixel[0];
		alpha=255;
		i_pixel+=1;
		break;
	case PF_nonStandard:
		break;
	}
}

// ----------------------------------------------------------------------

inline void Image::copyPixel(uint8 *&o_pixel, PixelFormat destPixelFormat, const uint8 *&i_pixel, PixelFormat sourcePixelFormat)
{
	uint8 r,g,b,a;
	getPixel(r,g,b,a,i_pixel,sourcePixelFormat);
	setPixel(o_pixel, destPixelFormat, r,g,b,a);
}

// ======================================================================

#endif
