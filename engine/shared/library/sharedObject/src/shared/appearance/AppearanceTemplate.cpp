// ======================================================================
//
// AppearanceTemplate.cpp
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/AppearanceTemplate.h"

#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/ExtentList.h"
#include "sharedCollision/Extent.h"
#include "sharedDebug/DataLint.h"
#include "sharedCollision/FloorManager.h"
#include "sharedCollision/FloorMesh.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Hardpoint.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/DataResourceList.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/Transform.h"

#include <algorithm>
#include <string>
#include <vector>

//-----------------------------------------------------------------

namespace AppearanceTemplateNamespace
{
	const Tag TAG_APPR = TAG (A,P,P,R);
	const Tag TAG_DTLA = TAG (D,T,L,A);
	const Tag TAG_HPTS = TAG (H,P,T,S);
	const Tag TAG_HPNT = TAG (H,P,N,T);
	const Tag TAG_FLOR = TAG (F,L,O,R);

	bool ms_verboseWarnings;

	char ms_crashReportInfo[MAX_PATH * 2];

	void remove();
}

using namespace AppearanceTemplateNamespace;

// ======================================================================
// PRIVATE AppearanceTemplate::PreloadManager
// ======================================================================

class AppearanceTemplate::PreloadManager
{
public:

	explicit PreloadManager (const AppearanceTemplate* appearanceTemplate);
	~PreloadManager ();

private:
	
	PreloadManager ();
	PreloadManager (const PreloadManager&);
	PreloadManager& operator= (const PreloadManager&);

private:

	const FloorMesh* m_preloadFloorMesh;
};

// ----------------------------------------------------------------------

AppearanceTemplate::PreloadManager::PreloadManager (const AppearanceTemplate* const appearanceTemplate) :
	m_preloadFloorMesh (0)
{
	if (appearanceTemplate->m_floorName)
		m_preloadFloorMesh = FloorMeshList::fetch (appearanceTemplate->m_floorName);
}

// ----------------------------------------------------------------------

AppearanceTemplate::PreloadManager::~PreloadManager ()
{
	if (m_preloadFloorMesh)
	{
		m_preloadFloorMesh->releaseReference ();
		m_preloadFloorMesh = 0;
	}
}

// ======================================================================
// Construct an AppearanceTemplate

AppearanceTemplate::AppearanceTemplate(const char *newName)
:	m_referenceCount(0),
	m_crcName(new CrcLowerString(newName)),
	m_extent(nullptr),
	m_collisionExtent(nullptr),
	m_hardpoints(nullptr),
	m_floorName(nullptr),
	m_preloadManager (0)
{
	//-- Save info on most recently constructed appearance template.
	IGNORE_RETURN(snprintf(ms_crashReportInfo, sizeof(ms_crashReportInfo) - 1, "AppearanceTemplate: %s\n", newName ? newName : "<nullptr>"));
	ms_crashReportInfo[sizeof(ms_crashReportInfo) - 1] = '\0';
}

// ----------------------------------------------------------------------
/**
 * Destroy an AppearanceTemplate.
 */

AppearanceTemplate::~AppearanceTemplate(void)
{
	delete m_crcName;

	ExtentList::release(m_extent);
	m_extent = nullptr;

	ExtentList::release(m_collisionExtent);
	m_collisionExtent = nullptr;

	if (m_hardpoints)
	{
		std::for_each(m_hardpoints->begin(), m_hardpoints->end(), PointerDeleter());
		delete m_hardpoints;
		m_hardpoints = 0;
	}

	delete [] m_floorName;

	if (m_preloadManager)
	{
		delete m_preloadManager;
		m_preloadManager = 0;
	}
}

// ----------------------------------------------------------------------

void AppearanceTemplate::install()
{
	AppearanceTemplateList::assignBinding(TAG_APPR, create);
	AppearanceTemplateList::assignBinding(TAG_DTLA, createDtla);

	ms_crashReportInfo[0] = '\0';
	CrashReportInformation::addDynamicText(ms_crashReportInfo);

	ExitChain::add(remove, "AppearanceTemplateList::remove");
}

// ----------------------------------------------------------------------

void AppearanceTemplateNamespace::remove()
{
	AppearanceTemplateList::removeBinding(TAG_APPR);
	CrashReportInformation::removeDynamicText(ms_crashReportInfo);
}

// ----------------------------------------------------------------------

AppearanceTemplate *AppearanceTemplate::create(const char *newName, Iff *iff)
{
	AppearanceTemplate * temp = new AppearanceTemplate(newName);
	temp->load(*iff);
	return temp;
}

// ----------------------------------------------------------------------

AppearanceTemplate *AppearanceTemplate::createDtla(const char *newName, Iff *iff)
{
	iff->enterForm(TAG_DTLA);
	Tag version = iff->getCurrentName();
	iff->enterForm(version);
	return create(newName, iff);
}

// ----------------------------------------------------------------------

/**
 * Create a unique Appearance for this AppearanceTemplate.
 * 
 * This routine should be overloaded in derived classes as appropriate.
 * 
 * Derived classes should not chain down to this routine.
 * 
 * @return An Appearance for the specified AppearanceTemplate
 */

Appearance *AppearanceTemplate::createAppearance() const
{
	return new Appearance(this);
}

// ----------------------------------------------------------------------

void AppearanceTemplate::load(Iff & iff)
{
	iff.enterForm(TAG_APPR);

#ifdef _DEBUG
		DEBUG_WARNING (DataLint::isEnabled () && iff.getCurrentName () < TAG_0003, ("AppearanceTemplate [%s] loading older version -- this asset will need to be reexported", getCrcName ().getString ()));
#endif

		switch (iff.getCurrentName())
		{
		case TAG_0001:
			load_0001(iff);
			break;

		case TAG_0002:
			load_0002(iff);
			break;

		case TAG_0003:
			load_0003(iff);
			break;

 		default:
			FATAL (true, ("AppearanceTemplate::load - unsupported format"));
			break;
		}

	iff.exitForm(TAG_APPR);
}

// ----------------------------------------------------------------------

void AppearanceTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		loadExtents(iff);
		loadHardpoints(iff);

	iff.exitForm(TAG_0001);
}

// ----------

void AppearanceTemplate::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);

		loadExtents(iff);
		loadHardpoints(iff);
		IGNORE_RETURN(loadFloors(iff));

	iff.exitForm(TAG_0002);
}

// ----------

void AppearanceTemplate::load_0003(Iff & iff)
{
	iff.enterForm(TAG_0003);

		loadExtents(iff);
		loadCollisionExtents(iff);
		loadHardpoints(iff);
		IGNORE_RETURN(loadFloors(iff));

	iff.exitForm(TAG_0003);
}

// ----------------------------------------------------------------------
/**
 * Load the extents out of an Iff.
 * 
 * @param iff  Iff containing the extent information
 */

void AppearanceTemplate::loadExtents (Iff & iff)
{
	m_extent = ExtentList::fetch(iff);
}

void AppearanceTemplate::loadCollisionExtents (Iff & iff)
{
	m_collisionExtent = ExtentList::fetch(iff);

	if(m_collisionExtent && !m_collisionExtent->validate())
	{
		DEBUG_WARNING(true,("AppearanceTemplate::loadCollisionExtents - Extent loaded from %s is invalid",iff.getFileName()));
	}
}

// ----------------------------------------------------------------------
/**
 * Load the hardpoints out of an Iff.
 */

void AppearanceTemplate::loadHardpoints (Iff& iff)
{
	// This enterForm call MUST be optional, as older versions of the exporter
	// just didn't write out this form if there weren't any hardpoints
	// (instead of writing an empty form)

	if(iff.enterForm(TAG_HPTS,true))
	{
		char        hardpointName[256];
		Transform   hardpointTransform;

		int nHardpoints = iff.getNumberOfBlocksLeft();

		if (m_hardpoints)
		{
			std::for_each(m_hardpoints->begin(), m_hardpoints->end(), PointerDeleter());
			m_hardpoints->clear();
		}
		else
		{
			m_hardpoints = new HardpointList();
			m_hardpoints->reserve(static_cast<HardpointList::size_type>(nHardpoints));
		}

		for (int i = 0; i < nHardpoints; ++i)
		{
			iff.enterChunk(TAG_HPNT);
				hardpointTransform = iff.read_floatTransform();
				iff.readRest_char(hardpointName, sizeof(hardpointName));
			iff.exitChunk(TAG_HPNT);

			m_hardpoints->push_back(new Hardpoint (hardpointName, hardpointTransform));
		}
		
		iff.exitForm(TAG_HPTS);

		//-- Sort the hardpoints by crc name.
		std::sort(m_hardpoints->begin(), m_hardpoints->end(), Hardpoint::LessNameComparator());
	}
}

// ----------------------------------------------------------------------

bool AppearanceTemplate::loadFloors( Iff & iff )
{
	iff.enterForm (TAG_FLOR);

		iff.enterChunk(TAG_DATA);

			bool hasFloor = iff.read_bool8();
			if(hasFloor)
				m_floorName = iff.read_string();

		iff.exitChunk(TAG_DATA);

	iff.exitForm(TAG_FLOR);

	return true;
}

//-------------------------------------------------------------------

char const * AppearanceTemplate::getFloorName() const
{
	return m_floorName;
}

// ----------------------------------------------------------------------
/**
 * Set a new Extent for this AppearanceTemplate.
 * 
 * The current extent will be released.
 * 
 * @param newExtent  New extent for this AppearanceTemplate
 */

void AppearanceTemplate::setExtent(const Extent *newExtent)
{
	ExtentList::release(m_extent);
	m_extent = newExtent;
}

// ----------------------------------------------------------------------

void AppearanceTemplate::setCollisionExtent ( Extent const * newExtent )
{
	ExtentList::release(m_collisionExtent);
	m_collisionExtent = newExtent;
}

// ----------------------------------------------------------------------
/**
 * retrieve the index of a hardpoint given the hardpoint name.
 * 
 * Hardpoint names are compared in a case-insensitive manner.
 * 
 * The argument "optional" controls activity when the hardpoint name is not
 * found.  The routine will FATAL if optional is false and the hardpoint name
 * is not found.  The routine will return -1 if optional is true and the
 * hardpoint name isn't found.
 * 
 * @return index of hardpoint with given name if found, -1 or FATAL otherwise (see remarks).
 */

int AppearanceTemplate::getHardpointIndex(CrcString const &hardpointName, bool optional) const
{
	//-- Handle no-hardpoint case.
	if (!m_hardpoints)
	{
		FATAL(!optional, ("getHardpointIndex(%s) called non-optionally on appearance template [%s] with no hardpoints.", hardpointName.getString(), m_crcName->getString()));
		return -1;
	}

	//-- Search for the hardpoint in Crc order.
	std::pair<HardpointList::iterator, HardpointList::iterator> const rangePair = std::equal_range(m_hardpoints->begin(), m_hardpoints->end(), hardpointName, Hardpoint::LessNameComparator());
	if (rangePair.first == rangePair.second)
	{
		FATAL(!optional, ("getHardpointIndex(%s) called non-optionally on appearance template [%s], hardpoint doesn't exist.", hardpointName.getString(), m_crcName->getString()));
		return -1;
	}

	return std::distance(m_hardpoints->begin(), rangePair.first);
}

//-------------------------------------------------------------------

const Hardpoint& AppearanceTemplate::getHardpoint(int index) const
{
	NOT_NULL(m_hardpoints);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getHardpointCount());

	return *((*m_hardpoints)[static_cast<HardpointList::size_type>(index)]);
}

// ----------------------------------------------------------------------

int AppearanceTemplate::getHardpointCount(void) const
{
	return m_hardpoints ? static_cast<int>(m_hardpoints->size()) : 0;
}

// ----------------------------------------------------------------------

void AppearanceTemplate::drawDebugShapes ( DebugShapeRenderer * renderer ) const
{
	UNREF(renderer);

#ifdef _DEBUG

    if(ConfigSharedCollision::getDrawExtents())
	{
		if(m_collisionExtent)
		{
			m_collisionExtent->drawDebugShapes(renderer);
		}
	}

#endif
}

// ----------------------------------------------------------------------

void AppearanceTemplate::preloadAssets() const
{
	if (!m_preloadManager)
		m_preloadManager = new PreloadManager (this);
}


// ----------------------------------------------------------------------

void AppearanceTemplate::preloadAssetsLight () const
{
}

// ----------------------------------------------------------------------

void AppearanceTemplate::garbageCollect() const
{
	if (m_preloadManager)
	{
		delete m_preloadManager;
		m_preloadManager = 0;
	}
}

// ----------------------------------------------------------------------
/**
 *Get the CrcLowerString for this AppearanceTemplate.
 */

const CrcLowerString &AppearanceTemplate::getCrcName() const
{
	return *m_crcName;
}

// ----------------------------------------------------------------------
/**
 *Get the name of this AppearanceTemplate.
 *
 *This routine may return nullptr.
 */

const char *AppearanceTemplate::getName() const
{
	return m_crcName->getString();
}

// ======================================================================

