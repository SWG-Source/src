// ======================================================================
//
// ConfigSharedImage.cpp
// copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedImage/FirstSharedImage.h"
#include "sharedImage/ConfigSharedImage.h"

#include "sharedFoundation/ConfigFile.h"

// ======================================================================

#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("SharedImage", #a,   (b)))

// ======================================================================

namespace ConfigSharedImageNamespace
{
	bool ms_sample;
}

using namespace ConfigSharedImageNamespace;

// ======================================================================

void ConfigSharedImage::install()
{
	KEY_BOOL(sample, false);
}

// ----------------------------------------------------------------------

bool ConfigSharedImage::getSample()
{
	return ms_sample;
}

// ======================================================================


