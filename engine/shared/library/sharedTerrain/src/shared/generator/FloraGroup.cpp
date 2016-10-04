//
// FloraGroup.cpp
// asommers 9-17-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/FloraGroup.h"

#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedUtility/FileName.h"

// ======================================================================
// FloraGroupNamespace
// ======================================================================

namespace FloraGroupNamespace
{
	FloraGroup::FamilyChildData const ms_defaultFamilyChildData;
}

using namespace FloraGroupNamespace;

//-------------------------------------------------------------------
//
// FloraGroup::Info
//
FloraGroup::Info::Info () :
	m_familyId (0),
	m_childChoice (0)
{
}

//-------------------------------------------------------------------
	
FloraGroup::Info::~Info ()
{
}

//-------------------------------------------------------------------

int FloraGroup::Info::getFamilyId () const
{
	return m_familyId;
}

//-------------------------------------------------------------------
	
void FloraGroup::Info::setFamilyId (int const familyId)
{
	m_familyId = static_cast<uint8> (familyId);
}

//-------------------------------------------------------------------

float FloraGroup::Info::getChildChoice () const
{
	return static_cast<uint8> (m_childChoice) / 255.f;
}

//-------------------------------------------------------------------

void FloraGroup::Info::setChildChoice (float childChoice)
{
	m_childChoice = static_cast<uint8> (childChoice * 255.f);
}

//-------------------------------------------------------------------
//
// FloraGroup::FamilyChildData
//
FloraGroup::FamilyChildData::FamilyChildData () :
	familyId (0),
	weight (0.f),
	appearanceTemplateName (0),
	shouldSway (false),
	period (0.1f),
	displacement (0.01f),
	alignToTerrain (false),
	shouldScale (false),
	minimumScale (1.f),
	maximumScale (1.f)
{
}

//-------------------------------------------------------------------

FloraGroup::FamilyChildData::~FamilyChildData ()
{
	appearanceTemplateName = 0;
}

//-------------------------------------------------------------------
//
// FloraGroup::Family
//
class FloraGroup::Family
{
private:

	int                                familyId;
	char*                              name;
	PackedRgb                          color;

	//-- family data
	float                              density;
	bool                               floats;

	//-- child data
	mutable ArrayList<FamilyChildData> childList;

private:

	void removeChild (int childIndex);

private:

	Family ();
	Family (const Family& rhs);             //lint -esym(754, Family::Family)
	Family& operator= (const Family& rhs);  //lint -esym(754, Family::operator=)

public:

	explicit Family (int newFamilyId);
	~Family ();

	int                   getNumberOfChildren () const;
	FamilyChildData const & getChild(int childIndex) const;
	FamilyChildData const & getChild(char const * name) const;

	int                   createFlora (float randomNumber0To1) const;

	const char*           getName () const;
	void                  setName (const char* newName);

	const PackedRgb&      getColor () const;
	void                  setColor (const PackedRgb& newColor);

	float                 getDensity () const;
	void                  setDensity (float newDensity);

	bool                  getFloats () const;
	void                  setFloats (bool newFloats);

	int                   getFamilyId () const;
	void                  setFamilyId (int newId);

	void                  addChild (const FamilyChildData& familyChildData);
	void                  removeChild (const FamilyChildData& familyChildData);
	void                  setChild (const FamilyChildData& familyChildData);

	void                  renameChild (const char* oldName, const char* newName);
};

//-------------------------------------------------------------------

FloraGroup::Family::Family (int newFamilyId) :
	familyId (newFamilyId),
	name (0),
	color (),
	density (1),
	floats (false),
	childList ()
{
}

//-------------------------------------------------------------------

FloraGroup::Family::~Family ()
{
	delete [] name;
	name = 0;

	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
		removeChild (i);

	childList.clear ();
}

//-------------------------------------------------------------------

int FloraGroup::Family::getNumberOfChildren () const
{
	return childList.getNumberOfElements ();
}

//-------------------------------------------------------------------

const char* FloraGroup::Family::getName () const
{
	return name;
}

//-------------------------------------------------------------------

void FloraGroup::Family::setName (const char* newName)
{
	if (name)
	{
		delete [] name;
		name = 0;
	}

	if (newName)
		name = DuplicateString (newName);
}

//-------------------------------------------------------------------

const PackedRgb& FloraGroup::Family::getColor () const
{
	return color;
}

//-------------------------------------------------------------------

void FloraGroup::Family::setColor (const PackedRgb& newColor)
{
	color = newColor;
}

//-------------------------------------------------------------------

int FloraGroup::Family::getFamilyId () const
{
	return familyId;
}

//-------------------------------------------------------------------

void FloraGroup::Family::setFamilyId (int newFamilyId)
{
	DEBUG_FATAL (familyId < 0 || familyId > 255, ("FloraGroup::Family::setFamilyId [%s]: familyId %i outside the legal range of 0..255", getName (), familyId));
	familyId = newFamilyId;
}

//-------------------------------------------------------------------

void FloraGroup::Family::addChild (const FamilyChildData& familyChildData)
{
	DEBUG_FATAL (familyId != familyChildData.familyId, ("adding child to wrong family"));

	//-- add the name, but not the appearance template. those will be loaded when the appearance is actually used
	FamilyChildData child = familyChildData;
	child.familyId = familyId;

	//-- fixup fubar'ed names for a version
	FileName baseName (FileName::P_none, familyChildData.appearanceTemplateName);
	baseName.stripPathAndExt ();

	FileName const satName (FileName::P_appearance, baseName, "sat");
	if (TreeFile::exists (satName))
	{
		FileName const baseSatName (FileName::P_none, baseName, "sat");
		child.appearanceTemplateName = DuplicateString (baseSatName);
	}
	else
	{
		FileName const prtName (FileName::P_appearance, baseName, "prt");
		if (TreeFile::exists (prtName))
		{
			FileName const basePrtName (FileName::P_none, baseName, "prt");
			child.appearanceTemplateName = DuplicateString (basePrtName);
		}
		else
		{
			FileName const aptName (FileName::P_appearance, baseName, "apt");
			if (TreeFile::exists (aptName))
			{
				FileName const baseAptName (FileName::P_none, baseName, "apt");
				child.appearanceTemplateName = DuplicateString (baseAptName);
			}
			else
				child.appearanceTemplateName = DuplicateString (familyChildData.appearanceTemplateName);
		}
	}

	childList.add (child);
}

//-------------------------------------------------------------------

void FloraGroup::Family::removeChild (int childIndex)
{
	if (childList [childIndex].appearanceTemplateName)
	{
		delete [] const_cast<char*> (childList [childIndex].appearanceTemplateName);
		childList [childIndex].appearanceTemplateName = 0;
	}
}

//-------------------------------------------------------------------

void FloraGroup::Family::removeChild (const FamilyChildData& familyChildData)
{
	//-- search for child
	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
		if (_stricmp (childList [i].appearanceTemplateName, familyChildData.appearanceTemplateName) == 0)
			break;

	DEBUG_FATAL (i >= childList.getNumberOfElements (), ("child not found"));

	//-- remove child from list
	removeChild (i);
	childList.removeIndexAndCompactList (i);
}
	
//-------------------------------------------------------------------

FloraGroup::FamilyChildData const & FloraGroup::Family::getChild(int const childIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, childIndex, childList.size());
	return childList[childIndex];
}

//-------------------------------------------------------------------
//
// FloraGroup
//
FloraGroup::FloraGroup () :
	familyList (),
	errorFamilyColor (255, 0, 255)
{
}

//-------------------------------------------------------------------

FloraGroup::~FloraGroup ()
{
	//-- delete family list
	removeAllFamilies ();
}

//-------------------------------------------------------------------

int FloraGroup::getFamilyId (int familyIndex) const
{
	return familyList [familyIndex]->getFamilyId ();
}

//-------------------------------------------------------------------

int FloraGroup::findFamilyIndex (int familyId) const
{
	int i;
	for (i = 0; i < familyList.getNumberOfElements (); i++)
		if (familyList [i]->getFamilyId () == familyId)
			return i;

	return -1;
}

//-------------------------------------------------------------------

void FloraGroup::removeAllFamilies ()
{
	int i;
	for (i = 0; i < familyList.getNumberOfElements (); i++)
	{
		delete familyList [i];
		familyList [i] = 0;
	}

	familyList.clear ();
}

//-------------------------------------------------------------------

void FloraGroup::addFamily (int familyId, const char* name, const PackedRgb& color)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex != -1, ("family with id %i already exists", familyId));

	if (familyIndex == -1)  //lint !e774  //-- if is always true
	{
		Family* family = new Family (familyId);
		family->setColor (color);
		family->setName (name);
		familyList.add (family);
	} //lint !e429  //-- family has not been freed or returned
} 

//-------------------------------------------------------------------

void FloraGroup::removeFamily (int familyId)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	delete familyList [familyIndex];
	familyList [familyIndex] = 0;

	familyList.removeIndexAndCompactList (familyIndex);
}

//-------------------------------------------------------------------

void FloraGroup::addChild (const FamilyChildData& familyData)
{
	int familyIndex = findFamilyIndex (familyData.familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyData.familyId));

	familyList [familyIndex]->addChild (familyData);
}

//-------------------------------------------------------------------

void FloraGroup::removeChild (const FamilyChildData& familyData)
{
	int familyIndex = findFamilyIndex (familyData.familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyData.familyId));

	familyList [familyIndex]->removeChild (familyData);
}

//-------------------------------------------------------------------

int FloraGroup::getNumberOfFamilies () const
{
	return familyList.getNumberOfElements ();
}

//-------------------------------------------------------------------

int FloraGroup::getNumberOfChildren (int familyIndex) const
{
	return familyList [familyIndex]->getNumberOfChildren ();
}

//-------------------------------------------------------------------

FloraGroup::FamilyChildData const & FloraGroup::getChild(int const familyIndex, int const childIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, familyIndex, familyList.size());
	return familyList[familyIndex]->getChild(childIndex);
}

//-------------------------------------------------------------------

FloraGroup::FamilyChildData const & FloraGroup::Family::getChild(char const * const childName) const
{
	//-- search for child
	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
		if (_stricmp (childList [i].appearanceTemplateName, childName) == 0)
			return getChild (i);

	DEBUG_FATAL (true, ("child %s not found", childName));
	return ms_defaultFamilyChildData;
}

//-------------------------------------------------------------------

const PackedRgb& FloraGroup::getFamilyColor (int familyId) const
{
	//-- search for the appearance in the family list
	int familyIndex = findFamilyIndex (familyId);
	if (familyIndex != -1)
		return familyList [familyIndex]->getColor ();

	//-- family id wasn't found, so log and return default appearance
	DEBUG_REPORT_LOG_PRINT (true, ("FloraGroup::getFamilyColorById - familyId %i not found, using error color.\n", familyId));

	return errorFamilyColor;
}

//-------------------------------------------------------------------

int FloraGroup::findFamilyIndex (const PackedRgb& desiredColor) const
{
	//-- no, so search list
	int i;
	for (i = 0; i < familyList.getNumberOfElements (); i++)
		if (familyList [i]->getColor () == desiredColor)
			return i;

	//-- item still not found in list
	return -1;
}

//-------------------------------------------------------------------

FloraGroup::FamilyChildData const & FloraGroup::createFlora(FloraGroup::Info const & fgi) const
{
	int const familyId = fgi.getFamilyId ();
	float const childChoice = fgi.getChildChoice ();
	DEBUG_FATAL (childChoice < 0.f || childChoice > 1.f, ("FloraGroup::createFlora: childChoice %1.2f out of bounds (0..1)", childChoice));

	//-- search for the appearance in the family list
	int familyIndex = findFamilyIndex (familyId);
	if (familyIndex != -1 && familyList [familyIndex]->getNumberOfChildren () > 0)
		return familyList [familyIndex]->getChild (familyList [familyIndex]->createFlora (childChoice));

	//-- family id wasn't found, so log and return default appearance
	DEBUG_REPORT_LOG_PRINT (true, ("FloraGroup::chooseFlora - familyId %i not found, using no appearance\n", familyId));
	return ms_defaultFamilyChildData;
}

//-------------------------------------------------------------------

const FloraGroup::Info FloraGroup::chooseFlora (int familyId) const
{
	Info fgi = getDefaultFlora ();

	//-- search for the appearance in the family list
	int familyIndex = findFamilyIndex (familyId);
	if (familyIndex != -1)
	{
		fgi.setFamilyId (familyId);
	}
	else
	{
		//-- family id wasn't found, so log and return default appearance
		DEBUG_REPORT_LOG_PRINT (true, ("FloraGroup::chooseFlora - familyId %i not found, using no appearance\n", familyId));
	}

	return fgi;
}

//-------------------------------------------------------------------

const FloraGroup::Info FloraGroup::chooseFlora (const PackedRgb& desiredColor) const
{
	Info fgi = getDefaultFlora ();

	//-- search for the Flora in the family list
	int familyIndex = findFamilyIndex (desiredColor);
	if (familyIndex != -1)
	{
		fgi.setFamilyId (familyList [familyIndex]->getFamilyId ());
	}
	else
	{
		//-- family id wasn't found, so log and return default Flora
		DEBUG_REPORT_LOG_PRINT (true, ("FloraGroup::chooseFlora - family with color %i %i %i not found, using default flora\n", desiredColor.r, desiredColor.g, desiredColor.b));
	}

	return fgi;
}

//-------------------------------------------------------------------

void FloraGroup::load (Iff& iff)
{
	//-- delete all old data
	removeAllFamilies ();

	//-- load new data
	if (iff.enterForm (TAG (F,G,R,P), true))
	{
		switch (iff.getCurrentName ())
		{
		case TAG_0001:
			load_0001 (iff);
			break;

		case TAG_0002:
			load_0002 (iff);
			break;

		case TAG_0003:
			load_0003 (iff);
			break;

		case TAG_0004:
			load_0004 (iff);
			break;

		case TAG_0005:
			load_0005 (iff);
			break;

		case TAG_0006:
			load_0006 (iff);
			break;

		case TAG_0007:
			load_0007 (iff);
			break;

		case TAG_0008:
			load_0008 (iff);
			break;

		default:
			DEBUG_FATAL (true, ("FloraGroup::load unknown tag"));
		}

		iff.exitForm ();
	}
}

//-------------------------------------------------------------------

void FloraGroup::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (F,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("FloraGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				iff.read_string (nameBuffer, 100);
				family->setName (nameBuffer);

				PackedRgb color;
				color.r = iff.read_uint8 ();
				color.g = iff.read_uint8 ();
				color.b = iff.read_uint8 ();
				family->setColor (color);

				int numberOfChildren = iff.read_int32 ();

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					//-- new appearance changes require this addition
					strcat (nameBuffer, ".msh");

					float weight = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId               = familyId;
					fcd.appearanceTemplateName = nameBuffer;
					fcd.weight                 = weight;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family has not been freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void FloraGroup::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (F,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("FloraGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				iff.read_string (nameBuffer, 100);
				family->setName (nameBuffer);

				PackedRgb color;
				color.r = iff.read_uint8 ();
				color.g = iff.read_uint8 ();
				color.b = iff.read_uint8 ();
				family->setColor (color);

				{
					const float unused = iff.read_float ();
					UNREF (unused);
				}

				{
					const int32 unused = iff.read_int32 ();
					UNREF (unused);
				}

				{
					const float unused = iff.read_float ();
					UNREF (unused);
				}

				int numberOfChildren = iff.read_int32 ();

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					//-- new appearance changes require this addition
					strcat (nameBuffer, ".msh");

					float weight = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId               = familyId;
					fcd.appearanceTemplateName = nameBuffer;
					fcd.weight                 = weight;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family has not been freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void FloraGroup::load_0003 (Iff& iff)
{
	iff.enterForm (TAG_0003);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (F,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("FloraGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				iff.read_string (nameBuffer, 100);
				family->setName (nameBuffer);

				PackedRgb color;
				color.r = iff.read_uint8 ();
				color.g = iff.read_uint8 ();
				color.b = iff.read_uint8 ();
				family->setColor (color);

				{
					const float unused = iff.read_float ();
					UNREF (unused);
				}

				{
					const int32 unused = iff.read_int32 ();
					UNREF (unused);
				}

				{
					const float unused = iff.read_float ();
					UNREF (unused);
				}

				int numberOfChildren = iff.read_int32 ();

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);
					float weight = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId               = familyId;
					fcd.appearanceTemplateName = nameBuffer;
					fcd.weight                 = weight;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family has not been freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void FloraGroup::load_0004 (Iff& iff)
{
	iff.enterForm (TAG_0004);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (F,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("FloraGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				iff.read_string (nameBuffer, 100);
				family->setName (nameBuffer);

				PackedRgb color;
				color.r = iff.read_uint8 ();
				color.g = iff.read_uint8 ();
				color.b = iff.read_uint8 ();
				family->setColor (color);

				family->setDensity (iff.read_float ());

				int numberOfChildren = iff.read_int32 ();

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);
					float weight = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId               = familyId;
					fcd.appearanceTemplateName = nameBuffer;
					fcd.weight                 = weight;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family has not been freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void FloraGroup::load_0005 (Iff& iff)
{
	iff.enterForm (TAG_0005);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (F,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("FloraGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				iff.read_string (nameBuffer, 100);
				family->setName (nameBuffer);

				PackedRgb color;
				color.r = iff.read_uint8 ();
				color.g = iff.read_uint8 ();
				color.b = iff.read_uint8 ();
				family->setColor (color);

				family->setDensity (iff.read_float ());

				int numberOfChildren = iff.read_int32 ();

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					const float weight       = iff.read_float ();
					const bool shouldSway   = iff.read_int32 () != 0;
					const float displacement = iff.read_float ();
					const float period       = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId               = familyId;
					fcd.appearanceTemplateName = nameBuffer;
					fcd.weight                 = weight;
					fcd.shouldSway             = shouldSway;
					fcd.displacement           = displacement;
					fcd.period                 = period;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family has not been freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void FloraGroup::load_0006 (Iff& iff)
{
	iff.enterForm (TAG_0006);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (F,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("FloraGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				iff.read_string (nameBuffer, 100);
				family->setName (nameBuffer);

				PackedRgb color;
				color.r = iff.read_uint8 ();
				color.g = iff.read_uint8 ();
				color.b = iff.read_uint8 ();
				family->setColor (color);

				family->setDensity (iff.read_float ());
				family->setFloats (iff.read_int32 () != 0);

				int numberOfChildren = iff.read_int32 ();

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					const float weight       = iff.read_float ();
					const bool shouldSway   = iff.read_int32 () != 0;
					const float displacement = iff.read_float ();
					const float period       = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId               = familyId;
					fcd.appearanceTemplateName = nameBuffer;
					fcd.weight                 = weight;
					fcd.shouldSway             = shouldSway;
					fcd.displacement           = displacement;
					fcd.period                 = period;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family has not been freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void FloraGroup::load_0007 (Iff& iff)
{
	iff.enterForm (TAG_0007);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (F,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("FloraGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				iff.read_string (nameBuffer, 100);
				family->setName (nameBuffer);

				PackedRgb color;
				color.r = iff.read_uint8 ();
				color.g = iff.read_uint8 ();
				color.b = iff.read_uint8 ();
				family->setColor (color);

				family->setDensity (iff.read_float ());
				family->setFloats (iff.read_int32 () != 0);

				int numberOfChildren = iff.read_int32 ();

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					const float weight         = iff.read_float ();
					const bool shouldSway     = iff.read_int32 () != 0;
					const float displacement   = iff.read_float ();
					const float period         = iff.read_float ();
					const bool alignToTerrain = iff.read_int32 () != 0;

					FamilyChildData fcd;
					fcd.familyId               = familyId;
					fcd.appearanceTemplateName = nameBuffer;
					fcd.weight                 = weight;
					fcd.shouldSway             = shouldSway;
					fcd.displacement           = displacement;
					fcd.period                 = period;
					fcd.alignToTerrain         = alignToTerrain;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family has not been freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void FloraGroup::load_0008 (Iff& iff)
{
	iff.enterForm (TAG_0008);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (F,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("FloraGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				iff.read_string (nameBuffer, 100);
				family->setName (nameBuffer);

				PackedRgb color;
				color.r = iff.read_uint8 ();
				color.g = iff.read_uint8 ();
				color.b = iff.read_uint8 ();
				family->setColor (color);

				family->setDensity (iff.read_float ());
				family->setFloats (iff.read_int32 () != 0);

				int numberOfChildren = iff.read_int32 ();

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					const float weight         = iff.read_float ();
					const bool shouldSway     = iff.read_int32 () != 0;
					const float displacement   = iff.read_float ();
					const float period         = iff.read_float ();
					const bool alignToTerrain = iff.read_int32 () != 0;
					const bool shouldScale    = iff.read_int32 () != 0;
					const float minimumScale   = iff.read_float ();
					const float maximumScale   = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId               = familyId;
					fcd.appearanceTemplateName = nameBuffer;
					fcd.weight                 = weight;
					fcd.shouldSway             = shouldSway;
					fcd.displacement           = displacement;
					fcd.period                 = period;
					fcd.alignToTerrain         = alignToTerrain;
					fcd.shouldScale            = shouldScale;
					fcd.minimumScale           = minimumScale;
					fcd.maximumScale           = maximumScale;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family has not been freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void FloraGroup::save (Iff& iff) const
{
	iff.insertForm (TAG (F,G,R,P));

		iff.insertForm (TAG_0008);

			int numberOfFamilies = getNumberOfFamilies ();
			int i;
			for (i = 0; i < numberOfFamilies; i++)
			{
				const Family* family = familyList [i];

				iff.insertChunk (TAG (F,F,A,M));
				iff.insertChunkData (static_cast<int32> (family->getFamilyId ()));
				iff.insertChunkString (family->getName ());
				iff.insertChunkData (static_cast<uint8> (family->getColor ().r));
				iff.insertChunkData (static_cast<uint8> (family->getColor ().g));
				iff.insertChunkData (static_cast<uint8> (family->getColor ().b));
				iff.insertChunkData (family->getDensity ());
				iff.insertChunkData (family->getFloats () ? static_cast<int32> (1) : static_cast<int32> (0));

				int numberOfChildren = getNumberOfChildren (i);
				iff.insertChunkData (numberOfChildren);

				int j;
				for (j = 0; j < numberOfChildren; j++)
				{
					const FamilyChildData fcd = getChild (i, j);

					iff.insertChunkString (fcd.appearanceTemplateName);
					iff.insertChunkData (fcd.weight);
					iff.insertChunkData (fcd.shouldSway ? static_cast<int32> (1) : static_cast<int32> (0));
					iff.insertChunkData (fcd.displacement);
					iff.insertChunkData (fcd.period);
					iff.insertChunkData (fcd.alignToTerrain ? static_cast<int32> (1) : static_cast<int32> (0));
					iff.insertChunkData (fcd.shouldScale ? static_cast<int32> (1) : static_cast<int32> (0));
					iff.insertChunkData (fcd.minimumScale);
					iff.insertChunkData (fcd.maximumScale);
				}

				iff.exitChunk (TAG (F,F,A,M));
			}

		iff.exitForm (TAG_0008);

	iff.exitForm (TAG (F,G,R,P));
}

//-------------------------------------------------------------------

int FloraGroup::getFamilyId (const PackedRgb& desiredColor) const
{
	//-- search for the appearance in the family list
	int familyIndex = findFamilyIndex (desiredColor);
	if (familyIndex != -1)
		return familyList [familyIndex]->getFamilyId ();

	//-- color wasn't found, so log and return default flora
	return 0;
}

//-------------------------------------------------------------------

int FloraGroup::getFamilyNumberOfChildren (int familyId) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getNumberOfChildren ();
}

//-------------------------------------------------------------------

FloraGroup::FamilyChildData const & FloraGroup::getFamilyChild(int const familyId, int const childIndex) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return getChild (familyIndex, childIndex);
}

//-------------------------------------------------------------------

FloraGroup::FamilyChildData const & FloraGroup::getFamilyChild(int const familyId, char const * const childName) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getChild (childName);
}

//-------------------------------------------------------------------

const char* FloraGroup::getFamilyName (int familyId) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getName ();
}

//-------------------------------------------------------------------

void FloraGroup::setFamilyName (int familyId, const char* newName)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setName (newName);
}

//-------------------------------------------------------------------

void FloraGroup::setFamilyColor (int familyId, const PackedRgb& newColor)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setColor (newColor);
}

//-------------------------------------------------------------------

void FloraGroup::setFamilyId (int familyIndex, int newFamilyId)
{
	familyList [familyIndex]->setFamilyId (newFamilyId);
}

//-------------------------------------------------------------------

const FloraGroup::Info FloraGroup::getDefaultFlora () const
{
	return Info ();
}

//-------------------------------------------------------------------

void FloraGroup::Family::renameChild (const char* oldName, const char* newName)
{
	//-- search for child
	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
		if (_stricmp (childList [i].appearanceTemplateName, oldName) == 0)
		{
			//-- delete the name
			delete [] const_cast<char*> (childList [i].appearanceTemplateName);

			//-- rename the child
			childList [i].appearanceTemplateName = DuplicateString (newName);
		}
}

//-------------------------------------------------------------------

void FloraGroup::renameChild (int familyId, const char* oldName, const char* newName)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->renameChild (oldName, newName);
}

//-------------------------------------------------------------------

int FloraGroup::Family::createFlora (float randomNumber0To1) const
{
	DEBUG_FATAL ((randomNumber0To1 < 0.f || randomNumber0To1 > 1.f), ("Random number out of bounds: %g", randomNumber0To1));

	float weightSum = 0.f;
	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
		weightSum += childList[i].weight;

	float randomNumber = randomNumber0To1 * weightSum;

	int radialIndex = 0;
	for (i = 0; i < childList.getNumberOfElements (); i++)
	{
		if (randomNumber <= childList[i].weight)
		{
			radialIndex = i;
			break;
		}
		else
			randomNumber -= childList[i].weight;
	}

	return radialIndex;
}

//-------------------------------------------------------------------

bool FloraGroup::hasFamily (int familyId) const
{
	return findFamilyIndex (familyId) != -1;
}

//-------------------------------------------------------------------

bool FloraGroup::hasFamily (const char* name) const
{
	int i;
	for (i = 0; i < familyList.getNumberOfElements (); i++)
		if (_stricmp (name, familyList [i]->getName ()) == 0)
			return true;

	return false;
}

//-------------------------------------------------------------------

int FloraGroup::getFamilyId (const char* name) const
{
	int i;
	for (i = 0; i < familyList.getNumberOfElements (); i++)
		if (_stricmp (name, familyList [i]->getName ()) == 0)
			return familyList [i]->getFamilyId ();
	
	DEBUG_FATAL (true, ("family with name %s not found", name));

	return -1;  //lint !e527  //unreachable
}

//-------------------------------------------------------------------

float FloraGroup::Family::getDensity () const
{
	return density;
}

//-------------------------------------------------------------------

void FloraGroup::Family::setDensity (float newDensity)
{
	density = newDensity;
}

//-------------------------------------------------------------------

float FloraGroup::getFamilyDensity (int familyId) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getDensity ();
}

//-------------------------------------------------------------------

void FloraGroup::setFamilyDensity (int familyId, float density)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setDensity (density);
}

//-------------------------------------------------------------------

bool FloraGroup::Family::getFloats () const
{
	return floats;
}

//-------------------------------------------------------------------

void FloraGroup::Family::setFloats (bool newFloats)
{
	floats = newFloats;
}

//-------------------------------------------------------------------

bool FloraGroup::getFamilyFloats (int familyId) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getFloats ();
}

//-------------------------------------------------------------------

void FloraGroup::setFamilyFloats (int familyId, bool floats)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setFloats (floats);
}

//-------------------------------------------------------------------

void FloraGroup::Family::setChild (const FamilyChildData& familyChildData) 
{
	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
	{
		if (strcmp (familyChildData.appearanceTemplateName, childList [i].appearanceTemplateName) == 0)
		{
			const char* appearanceTemplateName = childList [i].appearanceTemplateName;

			childList [i] = familyChildData;
			childList [i].appearanceTemplateName = appearanceTemplateName;

			return;
		}
	}

	DEBUG_FATAL (true, ("child %s not found in family", familyChildData.appearanceTemplateName));
}

//-------------------------------------------------------------------

void FloraGroup::setFamilyChild (int familyId, const FamilyChildData& familyChildData) 
{
	const int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setChild (familyChildData);
}

//-------------------------------------------------------------------

