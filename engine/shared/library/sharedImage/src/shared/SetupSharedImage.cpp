// ======================================================================
//
// SetupSharedImage.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedImage/FirstSharedImage.h"
#include "sharedImage/SetupSharedImage.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedImage/ConfigSharedImage.h"
#include "sharedImage/ImageFormatList.h"
#include "sharedImage/ImageManipulation.h"
#include "sharedImage/TargaFormat.h"
#include "sharedDebug/InstallTimer.h"

// ======================================================================

bool         SetupSharedImage::ms_installed;
TargaFormat *SetupSharedImage::ms_targaFormat;

// ======================================================================

void SetupSharedImage::install(const Data &data)
{
	InstallTimer const installTimer("SetupSharedImage::install");

	ConfigSharedImage::install();

	DEBUG_FATAL(ms_installed, ("SetupSharedImage already installed"));

	ImageFormatList::install();
	ImageManipulation::InstallData  imInstallData;

	// -TRF- have ImagePlatform install platform-specific handlers into imInstallData here
	ImageManipulation::install(imInstallData);

	ms_installed = true;
	ExitChain::add(remove, "SetupSharedImage");

	// add support for specified formats
	if (data.m_supportTarga)
	{
		ms_targaFormat = new TargaFormat();
		ImageFormatList::addImageFormat(ms_targaFormat);
	}
}

// ----------------------------------------------------------------------

void SetupSharedImage::remove()
{
	DEBUG_FATAL(!ms_installed, ("SetupSharedImage not installed"));

	// remove supported formats
	if (ms_targaFormat)
	{
		ImageFormatList::removeImageFormat(ms_targaFormat);
		delete ms_targaFormat;
		ms_targaFormat = 0;
	}
}

// ----------------------------------------------------------------------

void SetupSharedImage::setupDefaultData(Data &data)
{
	data.m_supportTarga = true;
}

// ======================================================================
