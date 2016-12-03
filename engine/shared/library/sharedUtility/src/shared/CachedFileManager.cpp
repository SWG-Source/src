// ======================================================================
// 
// CachedFileManager.cpp
// asommers
// 
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/CachedFileManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedUtility/ConfigSharedUtility.h"

#include <algorithm>
#include <map>
#include <string>

// ======================================================================

namespace CachedFileManagerNamespace
{
#if PRODUCTION == 0

	class CachedFileInfo
	{
	public:

		explicit CachedFileInfo (char const * extension);
		~CachedFileInfo ();

		CrcString const * getExtension () const;
		int getSize () const;
		void addFile (int size);
		void report () const;

	private:

		CachedFileInfo ();
		CachedFileInfo (CachedFileInfo const &);
		CachedFileInfo & operator= (CachedFileInfo const &);

	private:

		PersistentCrcString const m_extension;
		int m_numberOfFiles;
		int m_size;
	};

	typedef std::map<CrcString const *, CachedFileInfo *, LessPointerComparator> ExtensionMap;
	ExtensionMap ms_extensionMap;

	int ms_totalAllocatedBytes;

#endif

	Tag const TAG_CACH = TAG (C,A,C,H);

	char * ms_filenames = nullptr;
	size_t ms_filenamesLength = 0;
	size_t ms_filenamesCurrentPos = 0;
	unsigned long ms_totalTime;
	size_t ms_numberOfFilesLoaded;

	std::string ms_cacheFileName;

	void remove ();
}

using namespace CachedFileManagerNamespace;

// ======================================================================

#if PRODUCTION == 0

CachedFileInfo::CachedFileInfo (char const * const extension) :
	m_extension (extension, false),
	m_numberOfFiles (0),
	m_size (0)
{
}

// ----------------------------------------------------------------------

CachedFileInfo::~CachedFileInfo ()
{
}

// ----------------------------------------------------------------------

CrcString const * CachedFileInfo::getExtension () const
{
	return &m_extension;
}

// ----------------------------------------------------------------------

int CachedFileInfo::getSize () const
{
	return m_size;
}

// ----------------------------------------------------------------------

void CachedFileInfo::addFile (int const size)
{
	++m_numberOfFiles;
	m_size += size;
}

// ----------------------------------------------------------------------

void CachedFileInfo::report () const
{
	REPORT_LOG (true, ("| type=%s, count=%5d, size=%9d\n", m_extension.getString (), m_numberOfFiles, m_size));
}

#endif

// ======================================================================

void CachedFileManager::install(bool const allowFileCaching)
{
	InstallTimer const installTimer("CachedFileManager::install");
	
	if (!allowFileCaching || ConfigSharedUtility::getDisableFileCaching())
		return;
	
	char const * const result = ConfigSharedUtility::getUseCacheFile();
	if (result)
	{
		ms_cacheFileName = result;

		Iff iff;
		if (iff.open (result, true))
		{
			iff.enterForm (TAG_CACH);
				iff.enterChunk (TAG_0000);

					//-- the entire chunk is filled with nullptr terminated strings
					ms_filenamesLength = iff.getChunkLengthLeft();
					ms_filenames = iff.readRest_char();

				iff.exitChunk (TAG_0000);

			iff.exitForm (TAG_CACH);
		}
	}

	ExitChain::add (CachedFileManagerNamespace::remove, "CachedFileManagerNamespace::remove");
}

// ----------------------------------------------------------------------

void CachedFileManagerNamespace::remove ()
{
	delete[] ms_filenames;
	ms_filenames = nullptr;
	ms_filenamesCurrentPos = 0;
	ms_filenamesLength = 0;

#if PRODUCTION == 0
	std::for_each (ms_extensionMap.begin (), ms_extensionMap.end (), PointerDeleterPairSecond ());
	ms_extensionMap.clear();
#endif
}

// ----------------------------------------------------------------------

void CachedFileManager::preloadSomeAssets ()
{
	if (!donePreloading ())
	{
		unsigned long const startTime = Clock::timeMs ();

		//-- preloading occurs in one second slices
		while (ms_filenamesCurrentPos < ms_filenamesLength && Clock::timeMs () - startTime < 1000)
		{
			char const * const fileName = ms_filenames + ms_filenamesCurrentPos;

			if (TreeFile::exists(fileName))
			{
#if PRODUCTION == 0
				int const fileSize = TreeFile::cacheFile (fileName);
				
				//-- record cached file information per extension
				char const * const possibleExtension = strrchr (fileName, '.');
				if (possibleExtension)
				{
					ConstCharCrcString const extension (possibleExtension + 1);
					ExtensionMap::iterator iter = ms_extensionMap.find ((const CrcString*)&extension);
					if (iter == ms_extensionMap.end ())
					{
						CachedFileInfo * const info = new CachedFileInfo (extension.getString ());
						std::pair<ExtensionMap::iterator, bool> result = ms_extensionMap.insert (ExtensionMap::value_type (info->getExtension (), info));
						DEBUG_FATAL (!result.second, ("insert failed"));
						iter = result.first;
					}
					
					iter->second->addFile (fileSize);
				}

				++ms_numberOfFilesLoaded;
#else
				IGNORE_RETURN(TreeFile::cacheFile (fileName));
#endif
			}

			ms_filenamesCurrentPos += strlen(fileName) + 1;
		}

#if PRODUCTION == 0
		unsigned long const stopTime = Clock::timeMs ();
		ms_totalTime += stopTime - startTime;
#endif

		if (donePreloading ())
		{
#if PRODUCTION == 0
			REPORT_LOG (true, ("CachedFileManager [%s]: Caching took [%.2f] seconds for [%d] files and increased memory usage by [%.2f] MB.\n", ms_cacheFileName.c_str (), static_cast<float> (ms_totalTime) / 1000.f, ms_numberOfFilesLoaded, static_cast<float> (ms_totalAllocatedBytes) / (1024.0f * 1024.0f)));

			int size = 0;
			while (!ms_extensionMap.empty ())
			{
				ExtensionMap::iterator iter = ms_extensionMap.begin ();
				iter->second->report ();
				size += iter->second->getSize ();
				delete iter->second;
				
				ms_extensionMap.erase (iter);
			}
			
			std::for_each(ms_extensionMap.begin (), ms_extensionMap.end (), PointerDeleterPairSecond ());
			ms_extensionMap.clear();
			REPORT_LOG (true, ("+ Total cached data size = %.2f MB.\n", static_cast<float> (size) / (1024.0f * 1024.0f)));
#endif

			delete[] ms_filenames;
			ms_filenames = nullptr;
			ms_filenamesCurrentPos = 0;
			ms_filenamesLength = 0;
		}
	}
}

// ----------------------------------------------------------------------

bool CachedFileManager::donePreloading ()
{
	return ms_filenames == nullptr || (ms_filenamesCurrentPos >= ms_filenamesLength);
}

// ----------------------------------------------------------------------

int CachedFileManager::getLoadingPercent ()
{
	if (ms_filenamesLength == 0)
		return 100;
	
	return (ms_filenamesCurrentPos * 100) / ms_filenamesLength;
}

// ======================================================================
