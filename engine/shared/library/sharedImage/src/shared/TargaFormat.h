// ======================================================================
//
// TargaFormat.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef TARGA_FORMAT_H
#define TARGA_FORMAT_H

// ======================================================================

#include "sharedImage/ImageFormat.h"
#include "sharedImage/Image.h"

// ======================================================================

namespace TargaHeaderNamespace
{
struct TargaHeader;
}
class TargaFormat: public ImageFormat
{
public:

	TargaFormat();
	virtual ~TargaFormat();

	virtual const char *getName() const;
	virtual bool        isValidImage(const char *filename) const;
	virtual bool        loadImage(const char *filename, Image **image) const;

	bool                loadImageReformat(const char *filename, Image **image, Image::PixelFormat format) const;

	virtual bool        saveImage(const Image &image, const char *filename);

	virtual int         getCommonExtensionCount() const;
	virtual const char *getCommonExtension(int index) const;

private:
	// disabled
	TargaFormat(const TargaFormat&);
	TargaFormat &operator =(const TargaFormat&);
};

// ======================================================================

#endif
