// ======================================================================
//
// ImageFormatList.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedImage/FirstSharedImage.h"
#include "sharedImage/ImageFormatList.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedImage/Image.h"
#include "sharedImage/ImageFormat.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

// ======================================================================

bool                                ImageFormatList::ms_installed;

ImageFormatList::ImageFormatVector *ImageFormatList::ms_imageFormats;
ImageFormatList::ExtensionMap      *ImageFormatList::ms_extensionMap;

// ======================================================================

void ImageFormatList::install()
{
	DEBUG_FATAL(ms_installed, ("ImageFormatList already installed"));

	ms_installed = true;
	ExitChain::add(remove, "ImageFormatList");

	ms_imageFormats = new ImageFormatVector();
	ms_extensionMap = new ExtensionMap();
}

// ----------------------------------------------------------------------

void ImageFormatList::remove()
{
	DEBUG_FATAL(!ms_installed, ("ImageFormatList not installed"));

	delete ms_extensionMap;
	ms_extensionMap = 0;

	// we don't own the image formats, so don't try to delete them
	delete ms_imageFormats;
	ms_imageFormats = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

void ImageFormatList::addImageFormat(const ImageFormat *imageFormat)
{
	DEBUG_FATAL(!ms_installed, ("ImageFormatList not installed"));
	NOT_NULL(ms_extensionMap);
	NOT_NULL(ms_imageFormats);
	NOT_NULL(imageFormat);

#ifdef _DEBUG
	//-- make sure image format isn't already installed
	const ImageFormatVector::const_iterator it = std::find(ms_imageFormats->begin(), ms_imageFormats->end(), imageFormat);
	DEBUG_FATAL(it != ms_imageFormats->end(), ("imageFormat [0x%08x, (%s)] already installed", imageFormat->getName()));
#endif

	//-- add image format to list of image formats
	ms_imageFormats->push_back(imageFormat);

	//-- add image format to extension map (allows us to try a format by file extension first)
	const int extensionCount = imageFormat->getCommonExtensionCount();
	for (int i = 0; i < extensionCount; ++i)
	{
		//-- get lowercase extension
		std::string extension = imageFormat->getCommonExtension(i);
		IGNORE_RETURN(std::transform(extension.begin(), extension.end(), extension.begin(), tolower));

		//-- map extension to this image format
		std::pair<ExtensionMap::iterator, bool> result = ms_extensionMap->insert(std::make_pair(extension, imageFormat));
		UNREF(result);
		DEBUG_REPORT_LOG(!result.second, ("image format [%s] has conflicting default extension [%s] with [%s]\n", imageFormat->getName(), extension.c_str(), (*result.first).second->getName()));
	}
}

// ----------------------------------------------------------------------

void ImageFormatList::removeImageFormat(const ImageFormat *imageFormat)
{
	DEBUG_FATAL(!ms_installed, ("ImageFormatList not installed"));
	NOT_NULL(ms_extensionMap);
	NOT_NULL(ms_imageFormats);
	NOT_NULL(imageFormat);

	//-- remove image format from extension map
	{
		ExtensionMap::iterator       it;
		ExtensionMap::const_iterator itEnd;
		do
		{
			it    = ms_extensionMap->begin();
			itEnd = ms_extensionMap->end();
			for (; it != itEnd; ++it)
			{
				if ((*it).second == imageFormat)
				{
					// found one, remove it
					ms_extensionMap->erase(it);

					// we invalidate the iterator once we modify the container, so restart search
					break;
				}
			}
		}
		while (it != itEnd);
	}

	//-- remove image format from image format list
	{
		// find the image format
		ImageFormatVector::iterator it = std::find(ms_imageFormats->begin(), ms_imageFormats->end(), imageFormat);
		DEBUG_FATAL(it == ms_imageFormats->end(), ("failed to find image format specified for removal"));
		
		// remove it
		// we don't own the formats, so don't erase it
		if (it != ms_imageFormats->end())
			IGNORE_RETURN(ms_imageFormats->erase(it));
	}
}

// ----------------------------------------------------------------------

int ImageFormatList::getImageFormatCount()
{
	DEBUG_FATAL(!ms_installed, ("ImageFormatList not installed"));
	NOT_NULL(ms_imageFormats);

	return static_cast<int>(ms_imageFormats->size());
}

// ----------------------------------------------------------------------

const ImageFormat *ImageFormatList::getImageFormat(int index)
{
	DEBUG_FATAL(!ms_installed, ("ImageFormatList not installed"));
	NOT_NULL(ms_imageFormats);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(ms_imageFormats->size()));

	return (*ms_imageFormats)[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

Image *ImageFormatList::loadImage(const char *filename, bool readOnly)
{
	DEBUG_FATAL(!ms_installed, ("ImageFormatList not installed"));
	NOT_NULL(filename);

	//-- first try to find format by the extension of the filename
	const char *extensionDot = strrchr(filename, '.');
	if (extensionDot)
	{
		// convert extension to lower case
		std::string extension(extensionDot+1);
		IGNORE_RETURN(std::transform(extension.begin(), extension.end(), extension.begin(), tolower));

		// see if we have a file format for this extension
		NOT_NULL(ms_extensionMap);
		const ExtensionMap::const_iterator it = ms_extensionMap->find(extension);
		if (it != ms_extensionMap->end())
		{
			// found a file format that thinks it knows how to open this file by its extension
			const ImageFormat *const imageFormat = (*it).second;
			NOT_NULL(imageFormat);

			Image *image = 0;
			const bool loadSuccess = imageFormat->loadImage(filename, &image);
			DEBUG_REPORT_LOG(!loadSuccess, ("default image format [%s] thinks image [%s] is invalid, trying alternate formats\n", imageFormat->getName(), filename));
			if (loadSuccess)
			{
				// loaded the image
				NOT_NULL(image);
				if (readOnly)
					image->setReadOnly();
				return image;
			}
		}
	}	
	// didn't find an image format that knows how to load by extension, so let's
	// try them all.  file might be saved as a non-standard extension.

	//-- find the first format that thinks it can load the format
	{
		NOT_NULL(ms_imageFormats);

		const size_t count = ms_imageFormats->size();
		for (size_t i = 0; i < count; ++i)
		{
			const ImageFormat *const imageFormat = (*ms_imageFormats)[i];

			if (imageFormat->isValidImage(filename))
			{
				// image format thinks it can load this
				Image *image = 0;
				const bool loadSuccess = imageFormat->loadImage(filename, &image);
				DEBUG_REPORT_LOG(!loadSuccess, ("image format [%s] thought it could load [%s] but failed, trying others\n", imageFormat->getName(), filename));
				if (loadSuccess)
				{
					// loaded the image
					NOT_NULL(image);
					if (readOnly)
						image->setReadOnly();
					return image;
				}
			}
		}
	}

	//-- failed
	return 0;
}

// ----------------------------------------------------------------------

Image *ImageFormatList::loadImage(const char *filename, int formatIndex, bool readOnly)
{
	DEBUG_FATAL(!ms_installed, ("ImageFormatList not installed"));

	//-- get the image format
	const ImageFormat *const imageFormat = getImageFormat(formatIndex);
	NOT_NULL(imageFormat);

	//-- load the image
	Image *image = 0;
	const bool loadSuccess = imageFormat->loadImage(filename, &image);
	DEBUG_REPORT_LOG(!loadSuccess, ("image format [%s] (format specified by user) failed to load [%s]\n", imageFormat->getName(), filename));
	if (loadSuccess)
	{
		// loaded the image
		NOT_NULL(image);
		if (readOnly)
			image->setReadOnly();
		return image;
	}
	else
	{
		// failed to load image
		return 0;
	}
}

// ======================================================================
