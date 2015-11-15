//===================================================================
//
// FractalGroup.cpp
// asommers 9-17-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/FractalGroup.h"

#include "sharedFile/Iff.h"
#include "sharedFractal/MultiFractal.h"
#include "sharedFractal/MultiFractalReaderWriter.h"

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
// FractalGroup::Family
//
class FractalGroup::Family
{
public:

	explicit Family (int familyId);
	~Family ();

	const char*         getName () const;
	void                setName (const char* name);

	int                 getFamilyId () const;
	void                setFamilyId (int familyId);

	const MultiFractal& getMultiFractal () const;
	MultiFractal&       getMultiFractal ();

private:

	Family ();
	Family (const Family& rhs);             //lint -esym (754, Family::Family)
	Family& operator= (const Family& rhs);  //lint -esym (754, Family::operator=)

private:

	char*               m_name;
	int                 m_familyId;
	MultiFractal        m_multiFractal;
};

//-------------------------------------------------------------------

FractalGroup::Family::Family (int familyId) :
	m_name (0),
	m_familyId (familyId),
	m_multiFractal ()
{
}

//-------------------------------------------------------------------

FractalGroup::Family::~Family ()
{
	delete [] m_name;
	m_name = 0;
}

//-------------------------------------------------------------------

const char* FractalGroup::Family::getName () const
{
	return m_name;
}

//-------------------------------------------------------------------

void FractalGroup::Family::setName (const char* name)
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

int FractalGroup::Family::getFamilyId () const
{
	return m_familyId;
}

//-------------------------------------------------------------------

void FractalGroup::Family::setFamilyId (int familyId)
{
	m_familyId = familyId;
}

//-------------------------------------------------------------------

const MultiFractal& FractalGroup::Family::getMultiFractal () const
{
	return m_multiFractal;
}

//-------------------------------------------------------------------

MultiFractal& FractalGroup::Family::getMultiFractal ()
{
	return m_multiFractal;  //lint !e1536  //-- exposing low access member
}

//===================================================================
//
// FractalGroup
//
FractalGroup::FractalGroup () :
	m_familyMap (new FamilyMap)
{
}

//-------------------------------------------------------------------

FractalGroup::~FractalGroup ()
{
	//-- delete family list
	reset ();

	delete m_familyMap;
	m_familyMap = 0;
}

//-------------------------------------------------------------------

void FractalGroup::load (Iff& iff)
{
	//-- delete family list first
	reset ();

	//-- load data
	if (iff.enterForm (TAG_MGRP, true))
	{
		DEBUG_WARNING (iff.getCurrentName () != TAG_0000, ("FractalGroup::load - loading old version"));

		switch (iff.getCurrentName ())
		{
		case TAG_0000:
			load_0000 (iff);
			break;

		default:
			DEBUG_FATAL (true, ("FractalGroup::load unknown tag"));
		}

		iff.exitForm ();
	}
}

//-------------------------------------------------------------------

void FractalGroup::save (Iff& iff) const
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

					iff.exitChunk ();

					MultiFractalReaderWriter::save (iff, family->getMultiFractal ());

				iff.exitForm ();
			}

		iff.exitForm ();

	iff.exitForm ();
}

//-------------------------------------------------------------------

void FractalGroup::reset ()
{
	for (FamilyMap::iterator iter = m_familyMap->begin (); iter != m_familyMap->end (); ++iter)
		delete iter->second;

	m_familyMap->clear ();
}

//-------------------------------------------------------------------

void FractalGroup::prepare (int cacheX, int cacheY)
{
	for (FamilyMap::iterator iter = m_familyMap->begin (); iter != m_familyMap->end (); ++iter)
		iter->second->getMultiFractal ().allocateCache (cacheX, cacheY);
}

//-------------------------------------------------------------------

const char* FractalGroup::getFamilyName (int familyId) const
{
	const Family* const family = getFamily (familyId);
	return family->getName ();
}

//-------------------------------------------------------------------

void FractalGroup::setFamilyName (int familyId, const char* name)
{
	Family* const family = getFamily (familyId);
	family->setName (name);
}

//-------------------------------------------------------------------

const MultiFractal* FractalGroup::getFamilyMultiFractal (int familyId) const
{
	const Family* const family = getFamily (familyId);
	return &family->getMultiFractal ();
}

//-------------------------------------------------------------------

MultiFractal* FractalGroup::getFamilyMultiFractal (int familyId)
{
	Family* const family = getFamily (familyId);
	return &family->getMultiFractal ();
}

//-------------------------------------------------------------------

void FractalGroup::addFamily (int familyId, const char* name)
{
	DEBUG_FATAL (getFamily (familyId) != 0, ("family with id %i already exists", familyId));

	Family* family = new Family (familyId);
	family->setName (name);

	IGNORE_RETURN (m_familyMap->insert (FamilyMap::value_type (familyId, family)));
}

//-------------------------------------------------------------------

void FractalGroup::removeFamily (int familyId)
{
	FamilyMap::iterator iter = m_familyMap->find (familyId);
	DEBUG_FATAL (iter == m_familyMap->end (), ("family with id %i not found", familyId));

	m_familyMap->erase (iter);
}

//-------------------------------------------------------------------

bool FractalGroup::hasFamily (int familyId) const
{
	return getFamily (familyId) != 0;
}

//-------------------------------------------------------------------

bool FractalGroup::findFamily (const char* name, int& familyId) const
{
	const Family* const family = getFamily (name);

	if (family)
		familyId = family->getFamilyId ();

	return family != 0;
}

//-------------------------------------------------------------------

int FractalGroup::getNumberOfFamilies () const
{
	return static_cast<int> (m_familyMap->size ());
}

//-------------------------------------------------------------------

const FractalGroup::Family* FractalGroup::getFamily (int familyId) const
{
	FamilyMap::const_iterator iter = m_familyMap->find (familyId);
	if (iter != m_familyMap->end ())
		return iter->second;

	return 0;
}

//-------------------------------------------------------------------

FractalGroup::Family* FractalGroup::getFamily (int familyId)
{
	FamilyMap::iterator iter = m_familyMap->find (familyId);
	if (iter != m_familyMap->end ())
		return iter->second;

	return 0;
}

//-------------------------------------------------------------------

const FractalGroup::Family* FractalGroup::getFamily (const char* familyName) const
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

FractalGroup::Family* FractalGroup::getFamily (const char* familyName)
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

void FractalGroup::setFamilyId (int familyIndex, int familyId)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, familyIndex, getNumberOfFamilies ());

	Family* const family = getFamily (familyIndex);
	NOT_NULL (family);
	family->setFamilyId (familyId);
}

//-------------------------------------------------------------------

void FractalGroup::load_0000 (Iff& iff)
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
					DEBUG_FATAL (getFamily (familyId) != 0, ("EnvironmentGroup::load familyId %i is already in use", familyId));

					char* nameBuffer = iff.read_string ();

					//-- add family
					addFamily (familyId, nameBuffer);

					delete [] nameBuffer;

				iff.exitChunk ();

				Family* const family = getFamily (familyId);
				NOT_NULL (family);
				MultiFractalReaderWriter::load (iff, family->getMultiFractal ());

			iff.exitForm ();
		}  //lint !e429  //-- family not freed or returned

	iff.exitForm ();
}

//-------------------------------------------------------------------

int FractalGroup::getFamilyId (int familyIndex) const
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

int FractalGroup::createFamily (const MultiFractal* multiFractal, const char* baseName)
{
	NOT_NULL (multiFractal);

	FamilyMap::iterator iter = m_familyMap->begin ();
	for (; iter != m_familyMap->end (); ++iter)
	{
		const Family* const family = iter->second;

		if (family->getMultiFractal () == *multiFractal)
			return family->getFamilyId ();
	}

	const int   familyId   = createUniqueFamilyId ();
	std::string familyName = createUniqueFamilyName (baseName);

	addFamily (familyId, familyName.c_str ());

	Family* const family = getFamily (familyId);
	NOT_NULL (family);
	family->getMultiFractal () = *multiFractal;

	return familyId;
}

//-------------------------------------------------------------------

int FractalGroup::createUniqueFamilyId () const
{
	int familyId = 1;

	while (getFamily (familyId) != 0)
		++familyId;

	return familyId;
}

//-------------------------------------------------------------------

std::string FractalGroup::createUniqueFamilyName (const char* baseName) const
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
