//
// EnvironmentGroup.cpp
// asommers 9-17-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/EnvironmentGroup.h"

#include "sharedFile/Iff.h"
#include "sharedTerrain/ColorRamp256.h"

//-------------------------------------------------------------------
//
// EnvironmentGroup::Info
//
EnvironmentGroup::Info::Info () :
	m_familyId(0)
{
}

//-------------------------------------------------------------------
	
EnvironmentGroup::Info::~Info ()
{
}

//-------------------------------------------------------------------

int EnvironmentGroup::Info::getFamilyId() const
{
	return m_familyId;
}

//-------------------------------------------------------------------
	
void EnvironmentGroup::Info::setFamilyId(int const familyId)
{
	DEBUG_WARNING(familyId < 0 || familyId > 255, ("EnvironmentGroup::Info::setFamilyId: familyId %i will be clamped to a uint8", familyId));
	m_familyId = static_cast<uint8>(familyId);
}

//-------------------------------------------------------------------
//
// EnvironmentGroup::Family
//
class EnvironmentGroup::Family
{
private:

	int                                 familyId;
	char*                               name;
	PackedRgb                           color;

	//-- family data
	float                               m_featherClamp;

private:

	Family ();
	Family (const Family& rhs);             //lint -esym (754, Family::Family)
	Family& operator= (const Family& rhs);  //lint -esym (754, Family::operator=)

public:

	explicit Family (int newFamilyId);
	~Family ();

	int                   getFamilyId () const;
	void                  setFamilyId (int newId);

	const char*           getName () const;
	void                  setName (const char* newName);

	const PackedRgb&      getColor () const;
	void                  setColor (const PackedRgb& newColor);

	float                 getFeatherClamp () const;
	void                  setFeatherClamp (float featherClamp);
};

//-------------------------------------------------------------------
	
EnvironmentGroup::Family::Family (int newFamilyId) :
	familyId (newFamilyId),
	name (0),
	color (),
	m_featherClamp (1.f)
{
}

//-------------------------------------------------------------------

EnvironmentGroup::Family::~Family ()
{
	delete [] name;
	name = 0;
}

//-------------------------------------------------------------------

void EnvironmentGroup::Family::setFamilyId (int newFamilyId)
{
	familyId = newFamilyId;
}

//-------------------------------------------------------------------

const char* EnvironmentGroup::Family::getName () const
{
	return name;
}

//-------------------------------------------------------------------

void EnvironmentGroup::Family::setName (const char* newName)
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

const PackedRgb& EnvironmentGroup::Family::getColor () const
{
	return color;
}

//-------------------------------------------------------------------

void EnvironmentGroup::Family::setColor (const PackedRgb& newColor)
{
	color = newColor;
}

//-------------------------------------------------------------------

int EnvironmentGroup::Family::getFamilyId () const
{
	return familyId;
}

//-------------------------------------------------------------------
//
// EnvironmentGroup
//
EnvironmentGroup::EnvironmentGroup () :
	familyList (),
	errorFamilyColor (255, 0, 255)
{
}

//-------------------------------------------------------------------

EnvironmentGroup::~EnvironmentGroup ()
{
	//-- delete family list
	removeAllFamilies ();
}

//-------------------------------------------------------------------

void EnvironmentGroup::setFamilyId (int familyIndex, int newFamilyId)
{
	familyList [familyIndex]->setFamilyId (newFamilyId);
}

//-------------------------------------------------------------------

const EnvironmentGroup::Info EnvironmentGroup::getDefaultEnvironment () const
{
	Info rfgi;
	rfgi.setFamilyId(0);

	return rfgi;
}

//-------------------------------------------------------------------

int EnvironmentGroup::findFamilyIndex (int familyId) const
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

int EnvironmentGroup::findFamilyIndex (const PackedRgb& desiredColor) const
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

void EnvironmentGroup::removeAllFamilies ()
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

void EnvironmentGroup::addFamily (int familyId, const char* name, const PackedRgb& color)
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

void EnvironmentGroup::removeFamily (int familyId)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	delete familyList [familyIndex];
	familyList [familyIndex] = 0;

	familyList.removeIndexAndCompactList (familyIndex);
}

//-------------------------------------------------------------------

int EnvironmentGroup::getNumberOfFamilies () const
{
	return familyList.getNumberOfElements ();
}

//-------------------------------------------------------------------

const EnvironmentGroup::Info EnvironmentGroup::chooseEnvironment (int familyId) const
{
	Info rfgi = getDefaultEnvironment ();

	//-- search for the shader in the family list
	int familyIndex = findFamilyIndex (familyId);
	if (familyIndex != -1)
		rfgi.setFamilyId(familyId);
	else
		//-- family id wasn't found, so log and return default shader
		DEBUG_REPORT_LOG_PRINT (true, ("EnvironmentGroup::chooseEnvironment - familyId %i not found, using default shader\n", familyId));

	return rfgi;
}

//-------------------------------------------------------------------

const EnvironmentGroup::Info EnvironmentGroup::chooseEnvironment (const PackedRgb& desiredColor) const
{
	Info rfgi = getDefaultEnvironment ();

	//-- search for the shader in the family list
	int familyIndex = findFamilyIndex (desiredColor);
	if (familyIndex != -1)
		rfgi.setFamilyId(familyList[familyIndex]->getFamilyId());
	else
		//-- family id wasn't found, so log and return default shader
		DEBUG_REPORT_LOG_PRINT (true, ("EnvironmentGroup::chooseEnvironment - family with color %i %i %i not found, using default shader\n", desiredColor.r, desiredColor.g, desiredColor.b));

	return rfgi;
}

//-------------------------------------------------------------------

void EnvironmentGroup::load (Iff& iff)
{
	//-- delete family list first
	removeAllFamilies ();

	//-- load data
	if (iff.enterForm (TAG (E,G,R,P), true))
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

		default:
			DEBUG_FATAL (true, ("EnvironmentGroup::load unknown tag"));
		}

		iff.exitForm ();
	}
}

//-------------------------------------------------------------------

void EnvironmentGroup::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterForm (TAG (E,F,A,M));

				iff.enterChunk (TAG_DATA);

					int familyId = iff.read_int32 ();

					//-- see if family id is already in use
					DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("EnvironmentGroup::load familyId %i is already in use", familyId));

					//-- add family
					Family* family = new Family (familyId);

					char* nameBuffer = iff.read_string ();
					family->setName (nameBuffer);
					delete [] nameBuffer;

					PackedRgb color;
					color.r = iff.read_uint8 ();
					color.g = iff.read_uint8 ();
					color.b = iff.read_uint8 ();
					family->setColor (color);

					family->setFeatherClamp (iff.read_float ());

				iff.exitChunk (true);

				familyList.add (family);

			iff.exitForm (true);
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void EnvironmentGroup::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterForm (TAG (E,F,A,M));

				iff.enterChunk (TAG_DATA);

					int familyId = iff.read_int32 ();

					//-- see if family id is already in use
					DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("EnvironmentGroup::load familyId %i is already in use", familyId));

					//-- add family
					Family* family = new Family (familyId);

					char* nameBuffer = iff.read_string ();
					family->setName (nameBuffer);
					delete [] nameBuffer;

					PackedRgb color;
					color.r = iff.read_uint8 ();
					color.g = iff.read_uint8 ();
					color.b = iff.read_uint8 ();
					family->setColor (color);

					family->setFeatherClamp (iff.read_float ());

				iff.exitChunk (true);

				familyList.add (family);

			iff.exitForm (true);
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void EnvironmentGroup::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterForm (TAG (E,F,A,M));

				iff.enterChunk (TAG_DATA);

					int familyId = iff.read_int32 ();

					//-- see if family id is already in use
					DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("EnvironmentGroup::load familyId %i is already in use", familyId));

					//-- add family
					Family* family = new Family (familyId);

					char* nameBuffer = iff.read_string ();
					family->setName (nameBuffer);
					delete [] nameBuffer;

					PackedRgb color;
					color.r = iff.read_uint8 ();
					color.g = iff.read_uint8 ();
					color.b = iff.read_uint8 ();
					family->setColor (color);

					family->setFeatherClamp (iff.read_float ());

				iff.exitChunk (true);
				
				familyList.add (family);

			iff.exitForm (true);
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void EnvironmentGroup::save (Iff& iff) const
{
	iff.insertForm (TAG (E,G,R,P));

		iff.insertForm (TAG_0002);

			int numberOfFamilies = getNumberOfFamilies ();
			int i;
			for (i = 0; i < numberOfFamilies; i++)
			{
				const Family* family = familyList [i];

				iff.insertForm (TAG (E,F,A,M));
					iff.insertChunk (TAG_DATA);

						iff.insertChunkData (static_cast<int32> (family->getFamilyId ()));
						iff.insertChunkString (family->getName ());
						iff.insertChunkData (static_cast<uint8> (family->getColor ().r));
						iff.insertChunkData (static_cast<uint8> (family->getColor ().g));
						iff.insertChunkData (static_cast<uint8> (family->getColor ().b));
						iff.insertChunkData (family->getFeatherClamp ());
					
					iff.exitChunk (TAG_DATA);
				iff.exitForm (TAG (E,F,A,M));
			}

		iff.exitForm (TAG_0002);

	iff.exitForm (TAG (E,G,R,P));
}

//-------------------------------------------------------------------

const PackedRgb& EnvironmentGroup::getFamilyColor (int familyId) const
{
	//-- search for the appearance in the family list
	int familyIndex = findFamilyIndex (familyId);
	if (familyIndex != -1)
		return familyList [familyIndex]->getColor ();

	//-- family id wasn't found, so log and return default shader
	DEBUG_REPORT_LOG_PRINT (true, ("EnvironmentGroup::getFamilyColorById - familyId %i not found, using error color.\n", familyId));
	return errorFamilyColor;
}

//-------------------------------------------------------------------

int EnvironmentGroup::getFamilyId (const PackedRgb& desiredColor) const
{
	//-- search for the appearance in the family list
	int familyIndex = findFamilyIndex (desiredColor);
	if (familyIndex != -1)
		return familyList [familyIndex]->getFamilyId ();

	//-- color wasn't found, so log and return default shader
	return 0;
}

//-------------------------------------------------------------------

const char* EnvironmentGroup::getFamilyName (int familyId) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getName ();
}

//-------------------------------------------------------------------

void EnvironmentGroup::setFamilyName (int familyId, const char* newName)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setName (newName);
}

//-------------------------------------------------------------------

void EnvironmentGroup::setFamilyColor (int familyId, const PackedRgb& newColor)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setColor (newColor);
}

//-------------------------------------------------------------------

int EnvironmentGroup::getFamilyId (int familyIndex) const
{
	return familyList [familyIndex]->getFamilyId ();
}

//-------------------------------------------------------------------

bool EnvironmentGroup::hasFamily (int familyId) const
{
	return findFamilyIndex (familyId) != -1;
}

//-------------------------------------------------------------------

bool EnvironmentGroup::hasFamily (const char* name) const
{
	int i;
	for (i = 0; i < familyList.getNumberOfElements (); i++)
		if (_stricmp (name, familyList [i]->getName ()) == 0)
			return true;

	return false;
}

//-------------------------------------------------------------------

int EnvironmentGroup::getFamilyId (const char* name) const
{
	int i;
	for (i = 0; i < familyList.getNumberOfElements (); i++)
		if (_stricmp (name, familyList [i]->getName ()) == 0)
			return familyList [i]->getFamilyId ();
	
	DEBUG_FATAL (true, ("family with name %s not found", name));

	return -1;  //lint !e527  //unreachable
}

//-------------------------------------------------------------------

float EnvironmentGroup::Family::getFeatherClamp () const
{
	return m_featherClamp;
}

//-------------------------------------------------------------------

void EnvironmentGroup::Family::setFeatherClamp (float featherClamp)
{
	m_featherClamp = featherClamp;
}

//-------------------------------------------------------------------

float EnvironmentGroup::getFamilyFeatherClamp (int familyId) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getFeatherClamp ();
}

//-------------------------------------------------------------------

void EnvironmentGroup::setFamilyFeatherClamp (int familyId, float featherClamp)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setFeatherClamp (featherClamp);
}

//-------------------------------------------------------------------

void EnvironmentGroup::copyFamily (const int destinationFamilyId, const int sourceFamilyId)
{
	setFamilyFeatherClamp (destinationFamilyId, getFamilyFeatherClamp (sourceFamilyId));
}

//===================================================================

