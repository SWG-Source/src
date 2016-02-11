//===================================================================
//
// BitmapGroup.cpp
//
// copyright 2004, sony online entertainment
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/BitmapGroup.h"

#include "sharedFile/Iff.h"
#include "sharedImage/Image.h"
#include "sharedImage/ImageFormatList.h"

#include <map>
#include <string>
#include <cstdio>

//===================================================================

namespace
{
	const Tag TAG_MGRP = TAG (M,G,R,P);
	const Tag TAG_MFAM = TAG (M,F,A,M);
}

//===================================================================
//
// BitmapGroup::Family
//
class BitmapGroup::Family
{
public:

	explicit Family (int familyId);
	~Family ();

	const char*         getName () const;
	void                setName (const char* name);
	void                setBitmapName (const char* name);
	const char*         getBitmapName() const;

	int                 getFamilyId () const;
	void                setFamilyId (int familyId);

	const Image& getBitmap () const;
	Image&       getBitmap ();
	void loadBitmap(const char* bitmapName);
	void loadBitmapByFilename();
	void setBitmap(const Image* image);

private:

	Family ();
	Family (const Family& rhs);             
	Family& operator= (const Family& rhs);  

private:

	char*               m_name;
	char*               m_bitmapName;
	int                 m_familyId;
	Image*              m_bitmap;
};

//-------------------------------------------------------------------

BitmapGroup::Family::Family (int familyId) :
	m_name (0),
	m_bitmapName (0),
	m_familyId (familyId),
	m_bitmap(0)
{


}

//-------------------------------------------------------------------

BitmapGroup::Family::~Family ()
{
	if(m_name)
	{
		delete [] m_name;
		m_name = 0;
	}
	if(m_bitmapName)
	{
		delete [] m_bitmapName;
		m_bitmapName = 0;
	}
	if(m_bitmap)
	{
		delete m_bitmap;
		m_bitmap = 0;
	}
}

//-------------------------------------------------------------------

void BitmapGroup::Family::loadBitmap(const char* bitmapName)
{
	if(m_bitmap)
	{
		delete m_bitmap;
		m_bitmap = 0;
	}

	bool   valid = false;
	char name[256];

	sprintf(name,"terrain/%s.tga",bitmapName);
	m_bitmap = ImageFormatList::loadImage (name);

	if (!m_bitmap)
	{
		DEBUG_WARNING (true, ("BitmapGroup::Family::loadBitmap - invalid image (%s)",name));
	}
	else
	{
		if(!(m_bitmap->getPixelFormat() == Image::PF_w_8))
		{
			DEBUG_WARNING (true, ("BitmapGroup::Family::loadBitmap %s - m_bitmap->getPixelFormat() returned %d - should be Image::PF_w_8",name,m_bitmap->getPixelFormat()));
		}
		else
		{
			valid = true;
		}
	}

	if (!valid)
	{
		delete m_bitmap;
		m_bitmap = 0;
	}
	else
	{
		setBitmapName(name);
	}
}

//-------------------------------------------------------------------

void BitmapGroup::Family::loadBitmapByFilename()
{

	if(m_bitmap)
	{
		delete m_bitmap;
		m_bitmap = 0;
	}

	bool   valid = false;

	m_bitmap = ImageFormatList::loadImage (this->m_bitmapName);

	if (!m_bitmap)
	{
		if(!m_bitmapName)
		{
			DEBUG_WARNING (true, ("BitmapGroup::Family::loadBitmapByFilename() - invalid image - m_bitmapName is nullptr"));
		}
		else
		{
			DEBUG_WARNING (true, ("BitmapGroup::Family::loadBitmapByFilename() - invalid image - %s",m_bitmapName));
		}
		
	}
	else
	{
		if(!(m_bitmap->getPixelFormat() == Image::PF_w_8))
		{
			DEBUG_WARNING (true, ("BitmapGroup::Family::loadBitmapByFilename() %s - m_bitmap->getPixelFormat() returned %d - should be Image::PF_w_8",this->m_bitmapName,m_bitmap->getPixelFormat()));
		}
		else
		{
			valid = true;
		}
	}

	if (!valid)
	{
		delete m_bitmap;
		m_bitmap = 0;
	}

	
}

//-------------------------------------------------------------------

const char* BitmapGroup::Family::getName () const
{
	return m_name;
}

//-------------------------------------------------------------------

void BitmapGroup::Family::setName (const char* name)
{
	if (m_name)
	{
		delete [] m_name;
		m_name = 0;
	}

	if (name)
		m_name = DuplicateString (name);
}

//-------------------------------------------------------------------

const char* BitmapGroup::Family::getBitmapName() const
{
	return m_bitmapName;
}

//-------------------------------------------------------------------

void BitmapGroup::Family::setBitmapName (const char* name)
{
	if (m_bitmapName)
	{
		delete [] m_bitmapName;
		m_bitmapName = 0;
	}

	if (name)
	{
		m_bitmapName = DuplicateString (name);
	}
}

//-------------------------------------------------------------------

void BitmapGroup::Family::setBitmap(const Image* image)
{
	if(m_bitmap)
	{
		delete m_bitmap;
		m_bitmap = 0;
	}
	m_bitmap = (Image*)image;
}

int BitmapGroup::Family::getFamilyId () const
{
	return m_familyId;
}

//-------------------------------------------------------------------

void BitmapGroup::Family::setFamilyId (int familyId)
{
	m_familyId = familyId;
}

//-------------------------------------------------------------------

const Image& BitmapGroup::Family::getBitmap () const
{
	return *m_bitmap;
}

//-------------------------------------------------------------------

Image& BitmapGroup::Family::getBitmap ()
{
	return *m_bitmap;  
}


BitmapGroup::BitmapGroup () :
	m_familyMap (new FamilyMap)
{
}

//-------------------------------------------------------------------

BitmapGroup::~BitmapGroup ()
{
	//-- delete family list
	reset ();

	delete m_familyMap;
	m_familyMap = 0;
}

//-------------------------------------------------------------------

void BitmapGroup::load (Iff& iff)
{
	//-- delete family list first
	reset ();

	//-- load data
	if (iff.enterForm (TAG_MGRP, true))
	{
		DEBUG_WARNING (iff.getCurrentName () != TAG_0000, ("BitmapGroup::load - loading old version"));

		switch (iff.getCurrentName ())
		{
		case TAG_0000:
			load_0000 (iff);
			break;

		default:
			DEBUG_FATAL (true, ("BitmapGroup::load unknown tag"));
		}

		iff.exitForm ();
	}
}

//-------------------------------------------------------------------

void BitmapGroup::save (Iff& iff) const
{
	iff.insertForm (TAG_MGRP);

		iff.insertForm (TAG_0000);

			for (FamilyMap::iterator iter = m_familyMap->begin (); iter != m_familyMap->end (); ++iter)
			{
				const Family* family = iter->second;

				iff.insertForm (TAG_MFAM);

					iff.insertChunk (TAG_DATA);

						iff.insertChunkData (static_cast<int32> (family->getFamilyId ()));
						iff.insertChunkString (family->getName ());
						iff.insertChunkString (family->getBitmapName());

					iff.exitChunk ();

				iff.exitForm ();
			}

		iff.exitForm ();

	iff.exitForm ();
}

//-------------------------------------------------------------------

void BitmapGroup::reset ()
{
	for (FamilyMap::iterator iter = m_familyMap->begin (); iter != m_familyMap->end (); ++iter)
		delete iter->second;

	m_familyMap->clear ();
}

//-------------------------------------------------------------------

const char* BitmapGroup::getFamilyName (int familyId) const
{
	const Family* const family = getFamily (familyId);
	return family->getName ();
}

//-------------------------------------------------------------------

void BitmapGroup::setFamilyName (int familyId, const char* name)
{
	Family* const family = getFamily (familyId);
	family->setName (name);
}

void BitmapGroup::loadFamilyBitmap(int familyId, const char* bitmapName)
{
	Family* const family = getFamily(familyId);
	family->loadBitmap(bitmapName);
}

//-------------------------------------------------------------------

const Image* BitmapGroup::getFamilyBitmap (int familyId) const
{
	const Family* const family = getFamily (familyId);
	return &family->getBitmap ();
}

//-------------------------------------------------------------------

Image* BitmapGroup::getFamilyBitmap (int familyId)
{

	Family* const family = getFamily (familyId);
	return &family->getBitmap ();
}

//-------------------------------------------------------------------

void BitmapGroup::reloadAllFamilyBitmaps()
{
	int i;
	for(i = 0; i < getNumberOfFamilies(); ++i)
	{
		const int id = getFamilyId(i);
		Family* const family = getFamily(id);
		NOT_NULL (family);
		family->loadBitmapByFilename();
	}
}

//-------------------------------------------------------------------

void BitmapGroup::addFamily (int familyId, const char* name, const char* bitmapName)
{
	DEBUG_FATAL (getFamily (familyId) != 0, ("family with id %i already exists", familyId));

	Family* family = new Family (familyId);
	family->setName (name);
	family->setBitmapName(bitmapName);

	IGNORE_RETURN (m_familyMap->insert (FamilyMap::value_type (familyId, family)));
}

//-------------------------------------------------------------------

void BitmapGroup::removeFamily (int familyId)
{
	FamilyMap::iterator iter = m_familyMap->find (familyId);
	DEBUG_FATAL (iter == m_familyMap->end (), ("family with id %i not found", familyId));

	Family* family = iter->second;
	delete family;
	m_familyMap->erase (iter);
}

//-------------------------------------------------------------------

bool BitmapGroup::hasFamily (int familyId) const
{
	return getFamily (familyId) != 0;
}

//-------------------------------------------------------------------

bool BitmapGroup::findFamily (const char* name, int& familyId) const
{
	const Family* const family = getFamily (name);

	if (family)
		familyId = family->getFamilyId ();

	return family != 0;
}

//-------------------------------------------------------------------

int BitmapGroup::getNumberOfFamilies () const
{
	return static_cast<int> (m_familyMap->size ());
}

//-------------------------------------------------------------------

const BitmapGroup::Family* BitmapGroup::getFamily (int familyId) const
{
	FamilyMap::const_iterator iter = m_familyMap->find (familyId);
	if (iter != m_familyMap->end ())
		return iter->second;

	return 0;
}

//-------------------------------------------------------------------

BitmapGroup::Family* BitmapGroup::getFamily (int familyId)
{
	FamilyMap::iterator iter = m_familyMap->find (familyId);
	if (iter != m_familyMap->end ())
		return iter->second;

	return 0;
}

//-------------------------------------------------------------------

const BitmapGroup::Family* BitmapGroup::getFamily (const char* familyName) const
{
	FamilyMap::iterator iter = m_familyMap->begin ();
	for (; iter != m_familyMap->end (); ++iter)
	{
		const Family* const family = iter->second;

		if (_stricmp (familyName, family->getName ()) == 0)
			return family;
	}

	return 0;
}

//-------------------------------------------------------------------

BitmapGroup::Family* BitmapGroup::getFamily (const char* familyName)
{
	FamilyMap::iterator iter = m_familyMap->begin ();
	for (; iter != m_familyMap->end (); ++iter)
	{
		Family* const family = iter->second;

		if (_stricmp (familyName, family->getName ()) == 0)
			return family;
	}

	return 0;
}

//-------------------------------------------------------------------

void BitmapGroup::setFamilyId (int familyIndex, int familyId)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, familyIndex, getNumberOfFamilies ());

	Family* const family = getFamily (familyIndex);
	NOT_NULL (family);
	family->setFamilyId (familyId);
}

//-------------------------------------------------------------------

void BitmapGroup::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterForm (TAG_MFAM);

				iff.enterChunk (TAG_DATA);

					int familyId = iff.read_int32 ();

					//-- see if family id is already in use
					DEBUG_FATAL (getFamily (familyId) != 0, ("BitmapGroup::load familyId %i is already in use", familyId));

					char* nameBuffer = iff.read_string ();
					char* bitmapNameBuffer = iff.read_string();

					//-- add family
					addFamily (familyId, nameBuffer, bitmapNameBuffer);
					Family* const family = getFamily(familyId);
					NOT_NULL (family);
					family->loadBitmapByFilename();
					delete [] nameBuffer;
					delete [] bitmapNameBuffer;

				iff.exitChunk ();

			

			iff.exitForm ();
		}  //lint !e429  //-- family not freed or returned

	iff.exitForm ();
}

//-------------------------------------------------------------------

int BitmapGroup::getFamilyId (int familyIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, familyIndex, static_cast<int> (m_familyMap->size ()));

	FamilyMap::iterator iter = m_familyMap->begin ();
	int i = 0;
	for (; i < familyIndex || iter == m_familyMap->end (); ++i, ++iter)
		;

	DEBUG_FATAL (iter == m_familyMap->end (), (""));
	return iter->second->getFamilyId ();
}


//-------------------------------------------------------------------

int BitmapGroup::createUniqueFamilyId () const
{
	int familyId = 1;

	while (getFamily (familyId) != 0)
		++familyId;

	return familyId;
}

//-------------------------------------------------------------------

std::string BitmapGroup::createUniqueFamilyName (const char* baseName) const
{
	char familyName [1000];
	int i = 0;

	do
	{
		++i;

		sprintf (familyName, "%s_%i", baseName, i);
	}
	while (getFamily (familyName) != 0);

	return std::string (familyName);
}

//===================================================================
