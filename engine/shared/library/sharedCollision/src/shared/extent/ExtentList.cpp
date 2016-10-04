// ======================================================================
//
// ExtentList.cpp
// copyright 1999 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/ExtentList.h"

#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CylinderExtent.h"
#include "sharedCollision/ComponentExtent.h"
#include "sharedCollision/DetailExtent.h"
#include "sharedCollision/MeshExtent.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFile/Iff.h"

#include <map>

const Tag TAG_NULL = TAG(N,U,L,L);

// ======================================================================

struct ExtentList::BindImpl
{
	typedef std::map<Tag, ExtentList::CreateFunction> Map_t;
	Map_t * m_map;
};

bool                  ExtentList::ms_installed;
ExtentList::BindImpl  ExtentList::ms_bindImpl;

// ----------

Extent * ExtentList::nullFactory ( Iff & iff )
{
	Tag name = iff.getCurrentName();

	DEBUG_FATAL(name != TAG_NULL,("ExtentList::nullFactory - Got an iff whose current tag isn't TAG_NULL"));
	UNREF(name);

	IGNORE_RETURN( iff.goForward() );

	return nullptr;
}

// ======================================================================
// Install the ExtentList system

void ExtentList::install()
{
	DEBUG_FATAL(ms_installed, ("already installed"));
	ms_installed = true;

	// ----------
	// create static data

	ms_bindImpl.m_map = new BindImpl::Map_t;

	// ----------
	// add exit chain entry

	ExitChain::add(ExtentList::remove, "ExtentList::remove", 0, false);

	// ----------
	// install dependencies

	Extent::install();
	BoxExtent::install();
	CylinderExtent::install();
	ComponentExtent::install();
	DetailExtent::install();
	MeshExtent::install();

	assignBinding(TAG_NULL,ExtentList::nullFactory);
}

// ----------------------------------------------------------------------
/**
 * Remove the ExtentList system.
 */

void ExtentList::remove(void)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	delete ms_bindImpl.m_map;
	ms_bindImpl.m_map = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------
/**
 * Assign a new CreateFunction to a Tag.
 * 
 * If the tag already exists in the binding list, the CreateFunction will
 * be changed to the new function.  If the tag is not in the binding list,
 * the Tag/CreateFunction pair will be added.
 * 
 * If the binding list is full and an attempt is made to add a new pair,
 * this routine will call Fatal in debug builds.
 * 
 * @param tag  Tag to bind to a CreateFunction
 * @param createFunction  Function to call to create an extent with this Tag
 */

void ExtentList::assignBinding(Tag tag, CreateFunction createFunction)
{
	DEBUG_FATAL(!createFunction, ("createFunction may not be nullptr"));

	(*ms_bindImpl.m_map) [tag] = createFunction;
}

// ----------------------------------------------------------------------
/**
 * Remove a Tag/CreateFunction binding.
 * 
 * No action is taken if the tag is not in the list.
 * 
 * @param tag  Tag for the binding to remove
 */

void ExtentList::removeBinding(Tag tag)
{
	IGNORE_RETURN (ms_bindImpl.m_map->erase (tag));
}

// ----------------------------------------------------------------------
/**
 * Fetch an Extent.
 * 
 * This routine may be passed nullptr.
 * 
 * This routine will increase the reference count of the specified extent.
 * 
 * @param extent  Extent to add a reference to
 */

const Extent *ExtentList::fetch(const Extent *extent)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	if (extent)
		IGNORE_RETURN( extent->incrementReference() );

	return extent;
}

// ----------------------------------------------------------------------
/**
 * Load an Extent from an Iff.
 * 
 * The reference count of this Extent will be 1.
 * 
 * @param iff  Data file containing the Extent
 */

Extent * ExtentList::create(Iff & iff)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	// handle no extents
	if (iff.atEndOfForm())
		return nullptr;

	const Tag tag = iff.getCurrentName();

	const BindImpl::Map_t::const_iterator it = ms_bindImpl.m_map->find (tag);
	
	if (it == ms_bindImpl.m_map->end ()) //lint !e1702 //operator is both
	{
		DEBUG_FATAL(true, ("binding not found"));
		return 0; //lint !e527 // unreachable
	}

	Extent *extent = (*it).second(iff); //lint !e1702 //operator is both

	return extent;
}

const Extent *ExtentList::fetch(Iff & iff)
{
	Extent *extent = create(iff); //lint !e1702 //operator is both

	// fetch the extent
	if(extent)
	{
		return fetch(extent);
	}
	else
	{
		return extent;
	}
}

// ----------------------------------------------------------------------
/**
 * Release an Extent.
 * 
 * This routine may be passed nullptr.
 * 
 * This routine will decrement the reference count of the Extent.  When
 * the reference count becomes 0, the Extent will be deleted.
 * 
 * @param extent  Extent that is no longer used
 */

void ExtentList::release(const Extent *extent)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	if (extent && extent->decrementReference() <= 0)
	{
		DEBUG_FATAL(extent->getReferenceCount() < 0, ("user count negative"));
		delete const_cast<Extent *>(extent);
	}
}

// ======================================================================
