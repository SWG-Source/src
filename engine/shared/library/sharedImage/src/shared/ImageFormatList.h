// ======================================================================
//
// ImageFormatList.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ImageFormatList_H
#define INCLUDED_ImageFormatList_H

// ======================================================================

class Image;
class ImageFormat;

// ======================================================================

class ImageFormatList
{
public:

	static void               install();

	static void               addImageFormat(const ImageFormat *imageFormat);
	static void               removeImageFormat(const ImageFormat *imageFormat);

	static int                getImageFormatCount();
	static const ImageFormat *getImageFormat(int index);

	static Image             *loadImage(const char *filename, bool readOnly = true);
	static Image             *loadImage(const char *filename, int formatIndex, bool readOnly = true);

private:

	static void               remove();

private:

	typedef std::vector<const ImageFormat*>            ImageFormatVector;
	typedef std::map<std::string, const ImageFormat*>  ExtensionMap;

private:

	static bool               ms_installed;

	static ImageFormatVector *ms_imageFormats;
	static ExtensionMap      *ms_extensionMap;

};

// ======================================================================

#endif
