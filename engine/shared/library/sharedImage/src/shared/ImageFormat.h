// ======================================================================
//
// ImageFormat.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef IMAGE_FORMAT
#define IMAGE_FORMAT

// ======================================================================

class Image;

// ======================================================================

class ImageFormat
{
public:

	ImageFormat();
	virtual ~ImageFormat();

	virtual const char *getName() const = 0;
	virtual bool        isValidImage(const char *filename) const = 0;
	virtual bool        loadImage(const char *filename, Image **image) const = 0;

	virtual int         getCommonExtensionCount() const = 0;
	virtual const char *getCommonExtension(int index) const = 0;

protected:

	static bool loadFileCreateBuffer(const char *filename, uint8 **buffer, int *bufferSize);

private:
	// disabled
	ImageFormat(const ImageFormat&);
	ImageFormat &operator =(const ImageFormat&);
};

// ======================================================================

#endif
