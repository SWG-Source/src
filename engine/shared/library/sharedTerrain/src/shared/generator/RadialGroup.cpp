//
// RadialGroup.cpp
// asommers 9-17-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/RadialGroup.h"

#include "sharedFile/Iff.h"

// ======================================================================
// RadialGroupNamespace
// ======================================================================

namespace RadialGroupNamespace
{
	RadialGroup::FamilyChildData const ms_defaultFamilyChildData;
}

using namespace RadialGroupNamespace;

//-------------------------------------------------------------------
//
// RadialGroup::Info
//
RadialGroup::Info::Info() :
	m_familyId(0),
	m_childChoice(0)
{
}

//-------------------------------------------------------------------
	
RadialGroup::Info::~Info()
{
}

//-------------------------------------------------------------------

int RadialGroup::Info::getFamilyId() const
{
	return m_familyId;
}

//-------------------------------------------------------------------
	
void RadialGroup::Info::setFamilyId(int const familyId)
{
	m_familyId = static_cast<uint8>(familyId);
}

//-------------------------------------------------------------------

float RadialGroup::Info::getChildChoice() const
{
	return static_cast<float>(m_childChoice) / 255.f;
}

//-------------------------------------------------------------------

void RadialGroup::Info::setChildChoice(float childChoice)
{
	m_childChoice = static_cast<uint8>(childChoice * 255.f);
}

//-------------------------------------------------------------------
//
// RadialGroup::FamilyChildData
//
RadialGroup::FamilyChildData::FamilyChildData () :
	familyId (0),
	weight (0.f),
	shaderTemplateName (0),
	distance (0),
	minWidth (0),
	maxWidth (0),
	minHeight (0),
	maxHeight (0),
	maintainAspectRatio(false),
	period (0),
	displacement (0),
	shouldSway (false),
	alignToTerrain (false),
	createPlus (false)
{
}

//-------------------------------------------------------------------

RadialGroup::FamilyChildData::~FamilyChildData ()
{
	shaderTemplateName = 0;
}

//-------------------------------------------------------------------
//
// RadialGroup::Family
//
class RadialGroup::Family
{
private:

	int                                 familyId;
	char*                               name;
	PackedRgb                           color;

	//-- family data
	float                               density;

	//-- child list
	mutable ArrayList<FamilyChildData>  childList;

private:

	void removeChild (int childIndex);

private:

	Family ();
	Family (const Family& rhs);             //lint -esym (754, Family::Family)
	Family& operator= (const Family& rhs);  //lint -esym (754, Family::operator=)

public:

	explicit Family (int newFamilyId);
	~Family ();

	int                   getNumberOfChildren () const;
	FamilyChildData const & getChild(int childIndex) const;
	FamilyChildData const & getChild(char const * name) const;

	int                   createRadial (float randomNumber0To1) const;

	const char*           getName () const;
	void                  setName (const char* newName);

	const PackedRgb&      getColor () const;
	void                  setColor (const PackedRgb& newColor);

	float                 getDensity () const;
	void                  setDensity (float newDensity);

	int                   getFamilyId () const;
	void                  setFamilyId (int newId);

	void                  addChild (const FamilyChildData& familyChildData);
	void                  removeChild (const FamilyChildData& familyChildData);
	void                  setChild (const FamilyChildData& familyChildData);

	void                  renameChild (const char* oldName, const char* newName);
};

//-------------------------------------------------------------------
	
RadialGroup::Family::Family (int newFamilyId) :
	familyId (newFamilyId),
	name (0),
	color (),
	density (1),
	childList ()
{
}

//-------------------------------------------------------------------

RadialGroup::Family::~Family ()
{
	delete [] name;
	name = 0;

	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
		removeChild (i);

	childList.clear ();
}

//-------------------------------------------------------------------

int RadialGroup::Family::getNumberOfChildren () const
{
	return childList.getNumberOfElements ();
}

//-------------------------------------------------------------------

void RadialGroup::Family::setFamilyId (int newFamilyId)
{
	familyId = newFamilyId;
}

//-------------------------------------------------------------------

const char* RadialGroup::Family::getName () const
{
	return name;
}

//-------------------------------------------------------------------

void RadialGroup::Family::setName (const char* newName)
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

const PackedRgb& RadialGroup::Family::getColor () const
{
	return color;
}

//-------------------------------------------------------------------

void RadialGroup::Family::setColor (const PackedRgb& newColor)
{
	color = newColor;
}

//-------------------------------------------------------------------

int RadialGroup::Family::getFamilyId () const
{
	return familyId;
}

//-------------------------------------------------------------------

int RadialGroup::Family::createRadial (float randomNumber0To1) const
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

void RadialGroup::Family::addChild (const FamilyChildData& familyChildData)
{
	DEBUG_FATAL (familyId != familyChildData.familyId, ("adding child to wrong family"));

	//-- add the name, but not the shader or shader template. those will be loaded when the shader is actually used
	FamilyChildData child = familyChildData;
	child.familyId = familyId;
	child.shaderTemplateName = DuplicateString (familyChildData.shaderTemplateName);

	childList.add (child);
}

//-------------------------------------------------------------------

void RadialGroup::Family::removeChild (int childIndex)
{
	if (childList [childIndex].shaderTemplateName)
	{
		delete [] const_cast<char*> (childList [childIndex].shaderTemplateName);
		childList [childIndex].shaderTemplateName = 0;
	}
}

//-------------------------------------------------------------------

void RadialGroup::Family::removeChild (const FamilyChildData& familyChildData)
{
	//-- search for child
	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
		if (_stricmp (childList [i].shaderTemplateName, familyChildData.shaderTemplateName) == 0)
			break;

	DEBUG_FATAL (i >= childList.getNumberOfElements (), ("child not found"));

	//-- remove child from list
	removeChild (i);
	childList.removeIndexAndCompactList (i);
}
	
//-------------------------------------------------------------------

RadialGroup::FamilyChildData const & RadialGroup::Family::getChild(int const childIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, childIndex, childList.size());
	return childList[childIndex];
}

//-------------------------------------------------------------------
//
// RadialGroup
//
RadialGroup::RadialGroup () :
	familyList (),
	errorFamilyColor (255, 0, 255)
{
}

//-------------------------------------------------------------------

RadialGroup::~RadialGroup ()
{
	//-- delete family list
	removeAllFamilies ();
}

//-------------------------------------------------------------------

void RadialGroup::setFamilyId (int familyIndex, int newFamilyId)
{
	familyList [familyIndex]->setFamilyId (newFamilyId);
}

//-------------------------------------------------------------------

const RadialGroup::Info RadialGroup::getDefaultRadial () const
{
	Info rfgi;
	rfgi.setFamilyId(0);
	rfgi.setChildChoice(0.f);

	return rfgi;
}

//-------------------------------------------------------------------

int RadialGroup::findFamilyIndex (int familyId) const
{
	//-- no, so search list
	int i;
	for (i = 0; i < familyList.getNumberOfElements (); i++)
		if (familyList [i]->getFamilyId () == familyId)
			return i;

	//-- item still not found in list
	return -1;
}

//-------------------------------------------------------------------

int RadialGroup::findFamilyIndex (const PackedRgb& desiredColor) const
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

void RadialGroup::removeAllFamilies ()
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

void RadialGroup::addFamily (int familyId, const char* name, const PackedRgb& color)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex != -1, ("family with id %i already exists", familyId));

	if (familyIndex == -1)  //lint !e774  //-- if always true
	{
		Family* family = new Family (familyId);
		family->setColor (color);
		family->setName (name);
		familyList.add (family);
	}  //lint !e429  //-- family not freed or returned
}

//-------------------------------------------------------------------

void RadialGroup::removeFamily (int familyId)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	delete familyList [familyIndex];
	familyList [familyIndex] = 0;

	familyList.removeIndexAndCompactList (familyIndex);
}

//-------------------------------------------------------------------

void RadialGroup::addChild (const FamilyChildData& familyData)
{
	int familyIndex = findFamilyIndex (familyData.familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyData.familyId));

	familyList [familyIndex]->addChild (familyData);
}

//-------------------------------------------------------------------

void RadialGroup::removeChild (const FamilyChildData& familyData)
{
	int familyIndex = findFamilyIndex (familyData.familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyData.familyId));

	familyList [familyIndex]->removeChild (familyData);
}

//-------------------------------------------------------------------

int RadialGroup::getNumberOfFamilies () const
{
	return familyList.getNumberOfElements ();
}

//-------------------------------------------------------------------

int RadialGroup::getNumberOfChildren (int familyIndex) const
{
	return familyList [familyIndex]->getNumberOfChildren ();
}

//-------------------------------------------------------------------

RadialGroup::FamilyChildData const & RadialGroup::getChild(int const familyIndex, int const childIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, familyIndex, familyList.size());
	return familyList[familyIndex]->getChild(childIndex);
}

//-------------------------------------------------------------------

RadialGroup::FamilyChildData const & RadialGroup::createRadial(RadialGroup::Info const & rgi) const
{
	int const familyId = rgi.getFamilyId ();
	float const childChoice = rgi.getChildChoice ();
	DEBUG_FATAL (childChoice < 0.f || childChoice > 1.f, ("RadialGroup::createRadial: childChoice %1.2f out of bounds (0..1)", childChoice));

	//-- search for the shader in the family list
	int familyIndex = findFamilyIndex (familyId);
	if (familyIndex != -1 && familyList [familyIndex]->getNumberOfChildren () > 0)
	{
		return familyList[familyIndex]->getChild(familyList[familyIndex]->createRadial(childChoice));
	}

	//-- family id wasn't found, so log and return default shader
	DEBUG_REPORT_LOG_PRINT (true, ("RadialGroup::chooseRadial - familyId %i not found, using default shader\n", familyId));
	return ms_defaultFamilyChildData;
}

//-------------------------------------------------------------------

const RadialGroup::Info RadialGroup::chooseRadial (int familyId) const
{
	Info rfgi = getDefaultRadial ();

	//-- search for the shader in the family list
	int familyIndex = findFamilyIndex (familyId);
	if (familyIndex != -1 && familyList [familyIndex]->getNumberOfChildren () > 0)
		rfgi.setFamilyId(familyId);
	else
		//-- family id wasn't found, so log and return default shader
		DEBUG_REPORT_LOG_PRINT (true, ("RadialGroup::chooseRadial - familyId %i not found, using default shader\n", familyId));

	return rfgi;
}

//-------------------------------------------------------------------

const RadialGroup::Info RadialGroup::chooseRadial (const PackedRgb& desiredColor) const
{
	Info rfgi = getDefaultRadial ();

	//-- search for the shader in the family list
	int familyIndex = findFamilyIndex (desiredColor);
	if (familyIndex != -1 && familyList [familyIndex]->getNumberOfChildren () > 0)
		rfgi.setFamilyId(familyList[familyIndex]->getFamilyId());
	else
		//-- family id wasn't found, so log and return default shader
		DEBUG_REPORT_LOG_PRINT (true, ("RadialGroup::chooseRadial - family with color %i %i %i not found, using default shader\n", desiredColor.r, desiredColor.g, desiredColor.b));

	return rfgi;
}

//-------------------------------------------------------------------

void RadialGroup::load (Iff& iff)
{
	//-- delete family list first
	removeAllFamilies ();

	//-- load data
	if (iff.enterForm (TAG (R,G,R,P), true))
	{
		switch (iff.getCurrentName ())
		{
		case TAG_0000:
			load_0000 (iff);
			break;

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


		default:
			DEBUG_FATAL (true, ("RadialGroup::load unknown tag"));
		}

		iff.exitForm ();
	}
}

//-------------------------------------------------------------------

void RadialGroup::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (R,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("RadialGroup::load familyId %i is already in use", familyId));

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
//				DEBUG_FATAL (numberOfChildren <= 0, ("family %i (id %i) has 0 children", i, familyId));

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					const float weight   = iff.read_float ();
					const float distance = iff.read_float ();
					const float width    = iff.read_float ();
					const float height   = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId           = familyId;
					fcd.shaderTemplateName = nameBuffer;
					fcd.weight             = weight;
					fcd.distance           = distance;
					fcd.minWidth           = width;
					fcd.maxWidth           = width;
					fcd.minHeight          = height;
					fcd.maxHeight          = height;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void RadialGroup::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (R,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("RadialGroup::load familyId %i is already in use", familyId));

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
//				DEBUG_FATAL (numberOfChildren <= 0, ("family %i (id %i) has 0 children", i, familyId));

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					const float weight   = iff.read_float ();
					const float distance = iff.read_float ();
					const float width    = iff.read_float ();
					const float height   = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId           = familyId;
					fcd.shaderTemplateName = nameBuffer;
					fcd.weight             = weight;
					fcd.distance           = distance;
					fcd.minWidth           = width;
					fcd.maxWidth           = width;
					fcd.minHeight          = height;
					fcd.maxHeight          = height;
					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void RadialGroup::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (R,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("RadialGroup::load familyId %i is already in use", familyId));

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
//				DEBUG_FATAL (numberOfChildren <= 0, ("family %i (id %i) has 0 children", i, familyId));

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					const float weight       = iff.read_float ();
					const float distance     = iff.read_float ();
					const float width        = iff.read_float ();
					const float height       = iff.read_float ();
					const bool shouldSway   = iff.read_int32 () != 0;
					const float displacement = iff.read_float ();
					const float period       = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId           = familyId;
					fcd.shaderTemplateName = nameBuffer;
					fcd.weight             = weight;
					fcd.distance           = distance;
					fcd.minWidth           = width;
					fcd.maxWidth           = width;
					fcd.minHeight          = height;
					fcd.maxHeight          = height;
					fcd.shouldSway         = shouldSway;
					fcd.displacement       = displacement;
					fcd.period             = period;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void RadialGroup::load_0003 (Iff& iff)
{
	iff.enterForm (TAG_0003);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (R,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("RadialGroup::load familyId %i is already in use", familyId));

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
//				DEBUG_FATAL (numberOfChildren <= 0, ("family %i (id %i) has 0 children", i, familyId));

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					const float weight         = iff.read_float ();
					const float distance       = iff.read_float ();
					const float width          = iff.read_float ();
					const float height         = iff.read_float ();
					const bool shouldSway     = iff.read_int32 () != 0;
					const float displacement   = iff.read_float ();
					const float period         = iff.read_float ();
					const bool alignToTerrain = iff.read_int32 () != 0;
					const bool createPlus     = iff.read_int32 () != 0;

					FamilyChildData fcd;
					fcd.familyId           = familyId;
					fcd.shaderTemplateName = nameBuffer;
					fcd.weight             = weight;
					fcd.distance           = distance;
					fcd.minWidth           = width;
					fcd.maxWidth           = width;
					fcd.minHeight          = height;
					fcd.maxHeight          = height;
					fcd.shouldSway         = shouldSway;
					fcd.displacement       = displacement;
					fcd.period             = period;
					fcd.alignToTerrain     = alignToTerrain;
					fcd.createPlus         = createPlus;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

void RadialGroup::load_0004 (Iff& iff)
{
	iff.enterForm (TAG_0004);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (R,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("RadialGroup::load familyId %i is already in use", familyId));

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
//				DEBUG_FATAL (numberOfChildren <= 0, ("family %i (id %i) has 0 children", i, familyId));

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					
					const float weight         = iff.read_float ();
					const float distance       = iff.read_float ();
					const float minWidth          = iff.read_float ();
					const float maxWidth          = iff.read_float ();
					const float minHeight         = iff.read_float ();
					const float maxHeight          = iff.read_float ();
					const bool maintainAspectRatio = iff.read_int32 () != 0;
					const bool shouldSway     = iff.read_int32 () != 0;
					const float displacement   = iff.read_float ();
					const float period         = iff.read_float ();
					const bool alignToTerrain = iff.read_int32 () != 0;
					const bool createPlus     = iff.read_int32 () != 0;

					FamilyChildData fcd;
					fcd.familyId           = familyId;
					fcd.shaderTemplateName = nameBuffer;
					fcd.weight             = weight;
					fcd.distance           = distance;
					
				
					fcd.minWidth           = minWidth;
					fcd.maxWidth           = maxWidth;
					fcd.minHeight          = minHeight;
					fcd.maxHeight          = maxHeight;
					fcd.shouldSway         = shouldSway;
					fcd.displacement       = displacement;
					fcd.period             = period;
					fcd.alignToTerrain     = alignToTerrain;
					fcd.createPlus         = createPlus;
					fcd.maintainAspectRatio = maintainAspectRatio;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void RadialGroup::save (Iff& iff) const
{
	iff.insertForm (TAG (R,G,R,P));

		iff.insertForm (TAG_0004);

			int numberOfFamilies = getNumberOfFamilies ();
			int i;
			for (i = 0; i < numberOfFamilies; i++)
			{
				const Family* family = familyList [i];

				iff.insertChunk (TAG (R,F,A,M));
				iff.insertChunkData (static_cast<int32> (family->getFamilyId ()));
				iff.insertChunkString (family->getName ());
				iff.insertChunkData (static_cast<uint8> (family->getColor ().r));
				iff.insertChunkData (static_cast<uint8> (family->getColor ().g));
				iff.insertChunkData (static_cast<uint8> (family->getColor ().b));
				iff.insertChunkData (family->getDensity ());

				int numberOfChildren = getNumberOfChildren (i);
				iff.insertChunkData (numberOfChildren);

				int j;
				for (j = 0; j < numberOfChildren; j++)
				{
					const FamilyChildData &fcd = getChild (i, j);

					iff.insertChunkString (fcd.shaderTemplateName);
					iff.insertChunkData (fcd.weight);
					iff.insertChunkData (fcd.distance);
					iff.insertChunkData (fcd.minWidth);
					iff.insertChunkData (fcd.maxWidth);
					iff.insertChunkData (fcd.minHeight);	
					iff.insertChunkData (fcd.maxHeight);
					iff.insertChunkData (fcd.maintainAspectRatio ? static_cast<int32> (1) : static_cast<int32> (0));
					iff.insertChunkData (fcd.shouldSway ? static_cast<int32> (1) : static_cast<int32> (0));
					iff.insertChunkData (fcd.displacement);
					iff.insertChunkData (fcd.period);
					iff.insertChunkData (fcd.alignToTerrain ? static_cast<int32> (1) : static_cast<int32> (0));
					iff.insertChunkData (fcd.createPlus ? static_cast<int32> (1) : static_cast<int32> (0));
				}

				iff.exitChunk (TAG (R,F,A,M));
			}

		iff.exitForm (TAG_0004);

	iff.exitForm (TAG (R,G,R,P));
}

//-------------------------------------------------------------------

const PackedRgb& RadialGroup::getFamilyColor (int familyId) const
{
	//-- search for the appearance in the family list
	int familyIndex = findFamilyIndex (familyId);
	if (familyIndex != -1)
		return familyList [familyIndex]->getColor ();

	//-- family id wasn't found, so log and return default shader
	DEBUG_REPORT_LOG_PRINT (true, ("RadialGroup::getFamilyColorById - familyId %i not found, using error color.\n", familyId));
	return errorFamilyColor;
}

//-------------------------------------------------------------------

int RadialGroup::getFamilyId (const PackedRgb& desiredColor) const
{
	//-- search for the appearance in the family list
	int familyIndex = findFamilyIndex (desiredColor);
	if (familyIndex != -1)
		return familyList [familyIndex]->getFamilyId ();

	//-- color wasn't found, so log and return default shader
	return 0;
}

//-------------------------------------------------------------------

void RadialGroup::Family::renameChild (const char* oldName, const char* newName)
{
	//-- search for child
	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
		if (_stricmp (childList [i].shaderTemplateName, oldName) == 0)
		{
			delete [] const_cast<char*> (childList [i].shaderTemplateName);
			childList [i].shaderTemplateName = DuplicateString (newName);
		}
}

//-------------------------------------------------------------------

void RadialGroup::Family::setChild (const FamilyChildData& familyChildData) 
{
	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
	{
		if (strcmp (familyChildData.shaderTemplateName, childList [i].shaderTemplateName) == 0)
		{
			const char* shaderTemplateName = childList [i].shaderTemplateName;

			childList [i] = familyChildData;
			childList [i].shaderTemplateName = shaderTemplateName;

			return;
		}
	}

	DEBUG_FATAL (true, ("child %s not found in family", familyChildData.shaderTemplateName));
}

//-------------------------------------------------------------------

void RadialGroup::renameChild (int familyId, const char* oldName, const char* newName)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->renameChild (oldName, newName);
}


//-------------------------------------------------------------------

int RadialGroup::getFamilyNumberOfChildren (int familyId) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getNumberOfChildren ();
}

//-------------------------------------------------------------------

const char* RadialGroup::getFamilyName (int familyId) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getName ();
}

//-------------------------------------------------------------------

RadialGroup::FamilyChildData const & RadialGroup::getFamilyChild(int familyId, int childIndex) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return getChild (familyIndex, childIndex);
}

//-------------------------------------------------------------------

void RadialGroup::setFamilyChild (int familyId, const FamilyChildData& familyChildData) 
{
	const int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setChild (familyChildData);
}

//-------------------------------------------------------------------

void RadialGroup::setFamilyName (int familyId, const char* newName)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setName (newName);
}

//-------------------------------------------------------------------

void RadialGroup::setFamilyColor (int familyId, const PackedRgb& newColor)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setColor (newColor);
}

//-------------------------------------------------------------------

int RadialGroup::getFamilyId (int familyIndex) const
{
	return familyList [familyIndex]->getFamilyId ();
}

//-------------------------------------------------------------------

RadialGroup::FamilyChildData const & RadialGroup::getFamilyChild(int familyId, char const * childName) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getChild (childName);
}
  
//-------------------------------------------------------------------
  
RadialGroup::FamilyChildData const & RadialGroup::Family::getChild(char const * const childName) const
{
	//-- search for child
	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
		if (_stricmp (childList [i].shaderTemplateName, childName) == 0)
			return getChild (i);

	DEBUG_FATAL (true, ("child %s not found", childName));
	return ms_defaultFamilyChildData;
}

//-------------------------------------------------------------------

bool RadialGroup::hasFamily (int familyId) const
{
	return findFamilyIndex (familyId) != -1;
}

//-------------------------------------------------------------------

bool RadialGroup::hasFamily (const char* name) const
{
	int i;
	for (i = 0; i < familyList.getNumberOfElements (); i++)
		if (_stricmp (name, familyList [i]->getName ()) == 0)
			return true;

	return false;
}

//-------------------------------------------------------------------

int RadialGroup::getFamilyId (const char* name) const
{
	int i;
	for (i = 0; i < familyList.getNumberOfElements (); i++)
		if (_stricmp (name, familyList [i]->getName ()) == 0)
			return familyList [i]->getFamilyId ();
	
	DEBUG_FATAL (true, ("family with name %s not found", name));

	return -1;  //lint !e527  //unreachable
}

//-------------------------------------------------------------------

float RadialGroup::Family::getDensity () const
{
	return density;
}

//-------------------------------------------------------------------

void RadialGroup::Family::setDensity (float newDensity)
{
	density = newDensity;
}

//-------------------------------------------------------------------

float RadialGroup::getFamilyDensity (int familyId) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getDensity ();
}

//-------------------------------------------------------------------

void RadialGroup::setFamilyDensity (int familyId, float density)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setDensity (density);
}

//-------------------------------------------------------------------

