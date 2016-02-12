//
// ShaderGroup.cpp
// asommers 9-17-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/ShaderGroup.h"

#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"

//-------------------------------------------------------------------
//
// ShaderGroup::Info
//
ShaderGroup::Info::Info () :
	m_priority (0),
	m_familyId (0),
	m_childChoice (0)
{
}

//-------------------------------------------------------------------
	
ShaderGroup::Info::~Info ()
{
}

//-------------------------------------------------------------------

int ShaderGroup::Info::getPriority () const
{
	return m_priority;
}

//-------------------------------------------------------------------
	
void ShaderGroup::Info::setPriority (int const priority)
{
	m_priority = static_cast<uint8> (priority);
}

//-------------------------------------------------------------------
	
int ShaderGroup::Info::getFamilyId () const
{
	return m_familyId;
}

//-------------------------------------------------------------------
	
void ShaderGroup::Info::setFamilyId (int const familyId)
{
	m_familyId = static_cast<uint8> (familyId);
}

//-------------------------------------------------------------------

float ShaderGroup::Info::getChildChoice () const
{
	return static_cast<uint8> (m_childChoice) / 255.f;
}

//-------------------------------------------------------------------

void ShaderGroup::Info::setChildChoice (float childChoice)
{
	m_childChoice = static_cast<uint8> (childChoice * 255.f);
}

//-------------------------------------------------------------------
//
// ShaderGroup::FamilyChildData
//
ShaderGroup::FamilyChildData::FamilyChildData () :
	familyId (0),
	shaderTemplateName (0),
	weight (0.f)
{
}

//-------------------------------------------------------------------

ShaderGroup::FamilyChildData::~FamilyChildData ()
{
	shaderTemplateName = 0;
}

//-------------------------------------------------------------------
//
// ShaderGroup::Family
//
class ShaderGroup::Family
{
private:

	int                       familyId;
	char*                     name;
	PackedRgb                 color;

	//-- family data
	float                     m_featherClamp;
	float                     shaderSize;
	char*                     surfacePropertiesName;
	const ObjectTemplate*     surfaceProperties;

	//-- child data
	mutable ArrayList<FamilyChildData>  childList;

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
	const FamilyChildData getChild (int childIndex) const;
	const FamilyChildData getChild (const char* name) const;
	void                  setChild (const FamilyChildData& familyChildData);

	int                   createShader (float randomNumber0To1) const;

	const char*           getName () const;
	void                  setName (const char* newName);

	const PackedRgb&      getColor () const;
	void                  setColor (const PackedRgb& newColor);

	float                 getFeatherClamp () const;
	void                  setFeatherClamp (float featherClamp);

	float                 getShaderSize () const;
	void                  setShaderSize (float meters);

	const char*           getSurfacePropertiesName () const;
	void                  setSurfacePropertiesName (const char* familySurfacePropertiesName);
	void                  loadSurfaceProperties ();
	const ObjectTemplate* getSurfaceProperties () const;

	int                   getFamilyId () const;
	void                  setFamilyId (int newId);

	void                  addChild (const FamilyChildData& familyChildData);
	void                  removeChild (const FamilyChildData& familyChildData);

	void                  renameChild (const char* oldName, const char* newName);
	float                 getChildWeight (const char* childName);
	void                  setChildWeight (const char* childName, float w);
};
	
//-------------------------------------------------------------------
	
ShaderGroup::Family::Family (int newFamilyId) :
	familyId (newFamilyId),
	name (0),
	color (),
	m_featherClamp (1.f),
	shaderSize (2.0f),
	surfacePropertiesName (0),
	surfaceProperties (0),
	childList ()
{
}

//-------------------------------------------------------------------

ShaderGroup::Family::~Family ()
{
	delete [] name;
	name = 0;

	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
		removeChild (i);

	childList.clear ();

	if (surfaceProperties)
	{
		const_cast<ObjectTemplate*> (surfaceProperties)->releaseReference ();
		surfaceProperties = 0;
	}

	delete [] surfacePropertiesName;
	surfacePropertiesName = 0;
}

//-------------------------------------------------------------------

int ShaderGroup::Family::getNumberOfChildren () const
{
	return childList.getNumberOfElements ();
}

//-------------------------------------------------------------------

void ShaderGroup::Family::setFamilyId (int newFamilyId)
{
	DEBUG_FATAL (familyId < 0 || familyId > 255, ("ShaderGroup::Family::setFamilyId [%s]: familyId %i outside the legal range of 0..255", getName (), familyId));
	familyId = newFamilyId;
}

//-------------------------------------------------------------------

const ShaderGroup::FamilyChildData ShaderGroup::Family::getChild (int childIndex) const
{
	FamilyChildData fcd;
	fcd.familyId           = familyId;
	fcd.shaderTemplateName = childList [childIndex].shaderTemplateName;
	fcd.weight             = childList [childIndex].weight;

	return fcd;
}

//-------------------------------------------------------------------

const char* ShaderGroup::Family::getName () const
{
	return name;
}

//-------------------------------------------------------------------

void ShaderGroup::Family::setName (const char* newName)
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

const char* ShaderGroup::Family::getSurfacePropertiesName () const
{
	return surfacePropertiesName;
}

//-------------------------------------------------------------------

const ObjectTemplate* ShaderGroup::Family::getSurfaceProperties () const
{
	return surfaceProperties;
}

//-------------------------------------------------------------------

void ShaderGroup::Family::setSurfacePropertiesName (const char* newSurfacePropertiesName)
{
	if (surfacePropertiesName)
	{
		delete [] surfacePropertiesName;
		surfacePropertiesName = 0;
	}

	if (newSurfacePropertiesName)
		surfacePropertiesName = DuplicateString (newSurfacePropertiesName);
}

//-------------------------------------------------------------------

const PackedRgb& ShaderGroup::Family::getColor () const
{
	return color;
}
	
//-------------------------------------------------------------------

void ShaderGroup::Family::setColor (const PackedRgb& newColor)
{
	color = newColor;
}

//-------------------------------------------------------------------

float ShaderGroup::Family::getShaderSize () const
{
	return shaderSize;
}

//-------------------------------------------------------------------

void ShaderGroup::Family::setShaderSize (float meters)
{
	shaderSize = meters;
}

//-------------------------------------------------------------------

int ShaderGroup::Family::getFamilyId () const
{
	return familyId;
}

//-------------------------------------------------------------------

int ShaderGroup::Family::createShader (float randomNumber0To1) const
{
	DEBUG_FATAL (randomNumber0To1 < 0.f || randomNumber0To1 > 1.f, ("Random number out of bounds: %g", randomNumber0To1));

	float weightSum = 0.f;
	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
		weightSum += childList[i].weight;

	float randomNumber = randomNumber0To1 * weightSum;

	int shaderIndex = 0;
	for (i = 0; i < childList.getNumberOfElements (); i++)
	{
		if (randomNumber <= childList[i].weight)
		{
			shaderIndex = i;
			break;
		}
		else
			randomNumber -= childList[i].weight;
	}

	return shaderIndex;
}

//-------------------------------------------------------------------

void ShaderGroup::Family::addChild (const FamilyChildData& familyChildData)
{
	DEBUG_FATAL (familyId != familyChildData.familyId, ("adding child to wrong family"));

	//-- add the name, but not the shader or shader template. those will be loaded when the shader is actually used
	FamilyChildData child = familyChildData;
	child.shaderTemplateName = DuplicateString (familyChildData.shaderTemplateName);

	childList.add (child);
}

//-------------------------------------------------------------------

void ShaderGroup::Family::removeChild (int childIndex)
{
	if (childList [childIndex].shaderTemplateName)
	{
		delete [] const_cast<char*> (childList [childIndex].shaderTemplateName);
		childList [childIndex].shaderTemplateName = 0;
	}
}

//-------------------------------------------------------------------

void ShaderGroup::Family::removeChild (const FamilyChildData& familyChildData)
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

const ShaderGroup::FamilyChildData ShaderGroup::Family::getChild (const char* childName) const
{
	//-- search for child
	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
		if (_stricmp (childList [i].shaderTemplateName, childName) == 0)
			return getChild (i);

	DEBUG_FATAL (true, ("child %s not found", childName));

	FamilyChildData fcd;  //lint !e527  //-- unreachable
	return fcd;           //lint !e527  //-- unreachable
}

//-------------------------------------------------------------------
//
// ShaderGroup
//
ShaderGroup::ShaderGroup () :
	familyList (),
	errorFamilyColor (255, 0, 255)
{
}

//-------------------------------------------------------------------

ShaderGroup::~ShaderGroup ()
{
	//-- delete family list
	removeAllFamilies ();
}

//-------------------------------------------------------------------

void ShaderGroup::setFamilyId (int familyIndex, int newFamilyId)
{
	familyList [familyIndex]->setFamilyId (newFamilyId);
}

//-------------------------------------------------------------------

const ShaderGroup::Info ShaderGroup::getDefaultShader () const
{
	return Info ();
}

//-------------------------------------------------------------------

int ShaderGroup::findFamilyIndex (int familyId) const
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

int ShaderGroup::findFamilyIndex (const PackedRgb& desiredColor) const
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

void ShaderGroup::removeAllFamilies ()
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

void ShaderGroup::addFamily (int familyId, const char* name, const PackedRgb& color)
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

void ShaderGroup::removeFamily (int familyId)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	delete familyList [familyIndex];
	familyList [familyIndex] = 0;

	familyList.removeIndexAndCompactList (familyIndex);
}

//-------------------------------------------------------------------

void ShaderGroup::addChild (const FamilyChildData& familyData)
{
	int familyIndex = findFamilyIndex (familyData.familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyData.familyId));

	familyList [familyIndex]->addChild (familyData);
}

//-------------------------------------------------------------------

void ShaderGroup::removeChild (const FamilyChildData& familyData)
{
	int familyIndex = findFamilyIndex (familyData.familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyData.familyId));

	familyList [familyIndex]->removeChild (familyData);
}

//-------------------------------------------------------------------

int ShaderGroup::getNumberOfFamilies () const
{
	return familyList.getNumberOfElements ();
}

//-------------------------------------------------------------------

int ShaderGroup::getNumberOfChildren (int familyIndex) const
{
	return familyList [familyIndex]->getNumberOfChildren ();
}

//-------------------------------------------------------------------

const ShaderGroup::FamilyChildData ShaderGroup::getChild (int familyIndex, int childIndex) const
{
	return familyList [familyIndex]->getChild (childIndex);
}

//-------------------------------------------------------------------

int ShaderGroup::createShader (const ShaderGroup::Info& sgi) const
{
	int const familyId = sgi.getFamilyId ();
	float const childChoice = sgi.getChildChoice ();
	DEBUG_FATAL (childChoice < 0.f || childChoice > 1.f, ("ShaderGroup::createShader: childChoice %1.2f out of bounds (0..1)", childChoice));

	//-- search for the shader in the family list
	int familyIndex = findFamilyIndex (familyId);
	if (familyIndex != -1 && familyList [familyIndex]->getNumberOfChildren () > 0)
		return familyList [familyIndex]->createShader (childChoice);

	//-- family id wasn't found, so log and return default shader
	DEBUG_REPORT_LOG_PRINT (true, ("ShaderGroup::chooseShader - familyId %i not found, using default shader\n", familyId));

	return 0;
}

//-------------------------------------------------------------------

const ShaderGroup::Info ShaderGroup::chooseShader (int familyId) const
{
	Info sgi = getDefaultShader ();

	//-- search for the shader in the family list
	int familyIndex = findFamilyIndex (familyId);
	if (familyIndex != -1 && familyList [familyIndex]->getNumberOfChildren () > 0)
	{
		sgi.setPriority (familyIndex);
		sgi.setFamilyId (familyId);
	}
	else
	{
		//-- family id wasn't found, so log and return default shader
		DEBUG_REPORT_LOG_PRINT (true, ("ShaderGroup::chooseShader - familyId %i not found, using default shader\n", familyId));
	}

	return sgi;
}

//-------------------------------------------------------------------

const ShaderGroup::Info ShaderGroup::chooseShader (const PackedRgb& desiredColor) const
{
	Info sgi = getDefaultShader ();

	//-- search for the shader in the family list
	int familyIndex = findFamilyIndex (desiredColor);
	if (familyIndex != -1 && familyList [familyIndex]->getNumberOfChildren () > 0)
	{
		sgi.setPriority (familyIndex);
		sgi.setFamilyId (familyList [familyIndex]->getFamilyId ());
	}
	else
	{
		//-- family id wasn't found, so log and return default shader
		DEBUG_REPORT_LOG_PRINT (true, ("ShaderGroup::chooseShader - family with color %i %i %i not found, using default shader\n", desiredColor.r, desiredColor.g, desiredColor.b));
	}

	return sgi;
}

//-------------------------------------------------------------------

void ShaderGroup::load (Iff& iff)
{
	//-- delete family list first
	removeAllFamilies ();

	//-- load data
	if (iff.enterForm (TAG (S,G,R,P), true))
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

		case TAG_0005:
			load_0005 (iff);
			break;

		case TAG_0006:
			load_0006 (iff);
			break;

		default:
			DEBUG_FATAL (true, ("ShaderGroup::load unknown tag"));
		}

		iff.exitForm ();
	}
}

//-------------------------------------------------------------------

void ShaderGroup::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (S,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("ShaderGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				family->setName ("nullptr");

				PackedRgb color;
				color.r = 255;
				color.g = 255;
				color.b = 255;
				family->setColor (color);

				family->setShaderSize (2.f);

				int numberOfChildren = iff.read_int32 ();
//				DEBUG_FATAL (numberOfChildren <= 0, ("family %i (id %i) has 0 children", i, familyId));

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					char nameBuffer [100];
					iff.read_string (nameBuffer, 100);

					FamilyChildData fcd;
					fcd.familyId           = familyId;
					fcd.shaderTemplateName = nameBuffer;
					fcd.weight             = 1.0f / static_cast<float>(numberOfChildren);

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void ShaderGroup::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (S,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("ShaderGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				iff.read_string (nameBuffer, 100);
				family->setName (nameBuffer);

				PackedRgb color;
				color.r = iff.read_uint8 ();
				color.g = iff.read_uint8 ();
				color.b = iff.read_uint8 ();
				family->setColor (color);

				family->setShaderSize (2.f);

				int numberOfChildren = iff.read_int32 ();
//				DEBUG_FATAL (numberOfChildren <= 0, ("family %i (id %i) has 0 children", i, familyId));

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					float weight = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId           = familyId;
					fcd.shaderTemplateName = nameBuffer;
					fcd.weight             = weight;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void ShaderGroup::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (S,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("ShaderGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				iff.read_string (nameBuffer, 100);
				family->setName (nameBuffer);

				PackedRgb color;
				color.r = iff.read_uint8 ();
				color.g = iff.read_uint8 ();
				color.b = iff.read_uint8 ();
				family->setColor (color);
				family->setShaderSize (static_cast<float> (iff.read_float ()));

				int numberOfChildren = iff.read_int32 ();
//				DEBUG_FATAL (numberOfChildren <= 0, ("family %i (id %i) has 0 children", i, familyId));

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					float weight = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId           = familyId;
					fcd.shaderTemplateName = nameBuffer;
					fcd.weight             = weight;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void ShaderGroup::load_0003 (Iff& iff)
{
	iff.enterForm (TAG_0003);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (S,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("ShaderGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				iff.read_string (nameBuffer, 100);
				family->setName (nameBuffer);

				PackedRgb color;
				color.r = iff.read_uint8 ();
				color.g = iff.read_uint8 ();
				color.b = iff.read_uint8 ();
				family->setColor (color);
				family->setShaderSize (iff.read_float ());
				
				const float unused = iff.read_float ();
				UNREF (unused);

				int numberOfChildren = iff.read_int32 ();
//				DEBUG_FATAL (numberOfChildren <= 0, ("family %i (id %i) has 0 children", i, familyId));

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					float weight = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId           = familyId;
					fcd.shaderTemplateName = nameBuffer;
					fcd.weight             = weight;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void ShaderGroup::load_0004 (Iff& iff)
{
	iff.enterForm (TAG_0004);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (S,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("ShaderGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				iff.read_string (nameBuffer, 100);
				family->setName (nameBuffer);

				PackedRgb color;
				color.r = iff.read_uint8 ();
				color.g = iff.read_uint8 ();
				color.b = iff.read_uint8 ();
				family->setColor (color);
				family->setShaderSize (iff.read_float ());
				family->setFeatherClamp (iff.read_float ());

				int numberOfChildren = iff.read_int32 ();

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					float weight = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId           = familyId;
					fcd.shaderTemplateName = nameBuffer;
					fcd.weight             = weight;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void ShaderGroup::load_0005 (Iff& iff)
{
	iff.enterForm (TAG_0005);

		char nameBuffer [100];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (S,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("ShaderGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				iff.read_string (nameBuffer, 100);
				family->setName (nameBuffer);

				PackedRgb color;
				color.r = iff.read_uint8 ();
				color.g = iff.read_uint8 ();
				color.b = iff.read_uint8 ();
				family->setColor (color);
				family->setShaderSize (iff.read_float ());
				family->setFeatherClamp (iff.read_float ());
				IGNORE_RETURN (iff.read_int32 ());

				int numberOfChildren = iff.read_int32 ();

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 100);

					float weight = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId           = familyId;
					fcd.shaderTemplateName = nameBuffer;
					fcd.weight             = weight;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void ShaderGroup::load_0006 (Iff& iff)
{
	iff.enterForm (TAG_0006);

		char nameBuffer [1000];

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG (S,F,A,M));

				int familyId = iff.read_int32 ();

				//-- see if family id is already in use
				DEBUG_FATAL (findFamilyIndex (familyId) != -1, ("ShaderGroup::load familyId %i is already in use", familyId));

				//-- add family
				Family* family = new Family (familyId);

				iff.read_string (nameBuffer, 1000);
				family->setName (nameBuffer);
				
				iff.read_string (nameBuffer, 1000);
				family->setSurfacePropertiesName (nameBuffer);

				PackedRgb color;
				color.r = iff.read_uint8 ();
				color.g = iff.read_uint8 ();
				color.b = iff.read_uint8 ();
				family->setColor (color);
				family->setShaderSize (iff.read_float ());
				family->setFeatherClamp (iff.read_float ());

				int numberOfChildren = iff.read_int32 ();

				int k;
				for (k = 0; k < numberOfChildren; k++)
				{
					iff.read_string (nameBuffer, 1000);

					float weight = iff.read_float ();

					FamilyChildData fcd;
					fcd.familyId           = familyId;
					fcd.shaderTemplateName = nameBuffer;
					fcd.weight             = weight;

					family->addChild (fcd);
				}

				familyList.add (family);

			iff.exitChunk ();
		}  //lint !e429  //-- family not freed or returned
		
	iff.exitForm ();
}

//-------------------------------------------------------------------

void ShaderGroup::save (Iff& iff) const
{
	iff.insertForm (TAG (S,G,R,P));

		iff.insertForm (TAG_0006);

			int numberOfFamilies = getNumberOfFamilies ();
			int i;
			for (i = 0; i < numberOfFamilies; i++)
			{
				const Family* family = familyList [i];

				iff.insertChunk (TAG (S,F,A,M));
				iff.insertChunkData (static_cast<int32> (family->getFamilyId ()));
				iff.insertChunkString (family->getName ());
				iff.insertChunkString (family->getSurfacePropertiesName () ? family->getSurfacePropertiesName () : "");
				iff.insertChunkData (static_cast<uint8> (family->getColor ().r));
				iff.insertChunkData (static_cast<uint8> (family->getColor ().g));
				iff.insertChunkData (static_cast<uint8> (family->getColor ().b));
				iff.insertChunkData (family->getShaderSize ());
				iff.insertChunkData (family->getFeatherClamp ());
				
				int numberOfChildren = getNumberOfChildren (i);
				iff.insertChunkData (numberOfChildren);

				int j;
				for (j = 0; j < numberOfChildren; j++)
				{
					const FamilyChildData fcd = getChild (i, j);

					iff.insertChunkString (fcd.shaderTemplateName);
					iff.insertChunkData (fcd.weight);
				}

				iff.exitChunk (TAG (S,F,A,M));
			}

		iff.exitForm (TAG_0006);

	iff.exitForm (TAG (S,G,R,P));
}

//-------------------------------------------------------------------

const PackedRgb& ShaderGroup::getFamilyColor (int familyId) const
{
	//-- search for the appearance in the family list
	int familyIndex = findFamilyIndex (familyId);
	if (familyIndex != -1)
		return familyList [familyIndex]->getColor ();

	//-- family id wasn't found, so log and return default shader
	DEBUG_REPORT_LOG_PRINT (true, ("ShaderGroup::getFamilyColorById - familyId %i not found, using error color.\n", familyId));
	return errorFamilyColor;
}

//-------------------------------------------------------------------

float ShaderGroup::getFamilyShaderSize (int familyId) const
{
	//-- search for the appearance in the family list
	int familyIndex = findFamilyIndex (familyId);
	if (familyIndex != -1)
		return familyList [familyIndex]->getShaderSize ();

	//-- family id wasn't found, so log and return default size
	DEBUG_REPORT_LOG_PRINT (true, ("ShaderGroup::getFamilyShaderSize - familyId %i not found, using default size.\n", familyId));
	return 2.f;
}

//-------------------------------------------------------------------

void ShaderGroup::setFamilyShaderSize (int familyId, float meters)
{
	//-- search for the appearance in the family list
	int familyIndex = findFamilyIndex (familyId);
	if (familyIndex != -1) 
		familyList [familyIndex]->setShaderSize (meters);
	else
		//-- family id wasn't found
		DEBUG_REPORT_LOG_PRINT (true, ("ShaderGroup::setFamilyShaderSize - familyId %i not found.\n", familyId));
}

//-------------------------------------------------------------------

int ShaderGroup::getFamilyId (const PackedRgb& desiredColor) const
{
	//-- search for the appearance in the family list
	int familyIndex = findFamilyIndex (desiredColor);
	if (familyIndex != -1)
		return familyList [familyIndex]->getFamilyId ();

	//-- color wasn't found, so log and return default shader
	return 0;
}

//-------------------------------------------------------------------

void ShaderGroup::Family::renameChild (const char* oldName, const char* newName)
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

float ShaderGroup::Family::getChildWeight (const char* childName)
{
	int i;
	for (i = 0; i < childList.getNumberOfElements (); i++)
		if (_stricmp (childList [i].shaderTemplateName, childName) == 0)
			return childList [i].weight;

	FATAL (true, ("Child %s not found", childName));

	return 0.f;  //lint !e527  //-- if always true
}

//------------------------------------------------------------------

void ShaderGroup::Family::setChildWeight (const char* childName, float w)
{
	bool found = false;
	for (int i = 0; i < childList.getNumberOfElements () && !found; i++)
		if (_stricmp (childList [i].shaderTemplateName, childName) == 0)
		{
			childList [i].weight = w;
			found = true;
		}

	FATAL (!found, ("Child %s not found", childName));
}

//-------------------------------------------------------------------

void ShaderGroup::renameChild (int familyId, const char* oldName, const char* newName)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->renameChild (oldName, newName);
}


//-------------------------------------------------------------------

void ShaderGroup::promoteFamily (int familyId)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	if (familyIndex>0)
	{
		Family* tempFamily        = familyList[familyIndex-1];
		familyList[familyIndex-1] = familyList[familyIndex];
		familyList[familyIndex]   = tempFamily;
	}
		
}

//-------------------------------------------------------------------

void ShaderGroup::demoteFamily (int familyId)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	if (familyIndex < familyList.getNumberOfElements ()-1)
	{
		Family* tempFamily        = familyList[familyIndex+1];
		familyList[familyIndex+1] = familyList[familyIndex];
		familyList[familyIndex]   = tempFamily;
	}
}

//-------------------------------------------------------------------

int ShaderGroup::getFamilyNumberOfChildren (int familyId) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getNumberOfChildren ();
}

//-------------------------------------------------------------------

const char* ShaderGroup::getFamilyName (int familyId) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getName ();
}

//-------------------------------------------------------------------

const ShaderGroup::FamilyChildData ShaderGroup::getFamilyChild (int familyId, int childIndex) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return getChild (familyIndex, childIndex);
}


//-------------------------------------------------------------------

void ShaderGroup::setFamilyName (int familyId, const char* newName)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setName (newName);
}

//-------------------------------------------------------------------

void ShaderGroup::setFamilyColor (int familyId, const PackedRgb& newColor)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setColor (newColor);
}

//-------------------------------------------------------------------

int ShaderGroup::getFamilyId (int familyIndex) const
{
	return familyList [familyIndex]->getFamilyId ();
}

//-------------------------------------------------------------------

bool ShaderGroup::hasFamily (int familyId) const
{
	return findFamilyIndex (familyId) != -1;
}

//-------------------------------------------------------------------

bool ShaderGroup::hasFamily (const char* name) const
{
	int i;
	for (i = 0; i < familyList.getNumberOfElements (); i++)
		if (_stricmp (name, familyList [i]->getName ()) == 0)
			return true;

	return false;
}

//-------------------------------------------------------------------

int ShaderGroup::getFamilyId (const char* name) const
{
	int i;
	for (i = 0; i < familyList.getNumberOfElements (); i++)
		if (_stricmp (name, familyList [i]->getName ()) == 0)
			return familyList [i]->getFamilyId ();
	
	DEBUG_FATAL (true, ("family with name %s not found", name));

	return -1;  //lint !e527  //unreachable
}

//-------------------------------------------------------------------

float ShaderGroup::Family::getFeatherClamp () const
{
	return m_featherClamp;
}

//-------------------------------------------------------------------

void ShaderGroup::Family::setFeatherClamp (float featherClamp)
{
	m_featherClamp = featherClamp;
}

//-------------------------------------------------------------------

float ShaderGroup::getFamilyFeatherClamp (int familyId) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getFeatherClamp ();
}

//-------------------------------------------------------------------

void ShaderGroup::setFamilyFeatherClamp (int familyId, float featherClamp)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setFeatherClamp (featherClamp);
}

//-------------------------------------------------------------------

const char* ShaderGroup::getFamilySurfacePropertiesName (int familyId) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getSurfacePropertiesName ();
}

//-------------------------------------------------------------------

void ShaderGroup::setFamilySurfacePropertiesName (int familyId, const char* surfacePropertiesName)
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setSurfacePropertiesName (surfacePropertiesName);
}

//-------------------------------------------------------------------

void ShaderGroup::Family::setChild (const FamilyChildData& familyChildData) 
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

const ShaderGroup::FamilyChildData ShaderGroup::getFamilyChild (int familyId, const char* childName) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getChild (childName);
}
  
//-------------------------------------------------------------------
  
void ShaderGroup::setFamilyChild (int familyId, const FamilyChildData& familyChildData) 
{
	const int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	familyList [familyIndex]->setChild (familyChildData);
}

//-------------------------------------------------------------------

const ObjectTemplate* ShaderGroup::getFamilySurfaceProperties (int familyId) const
{
	int familyIndex = findFamilyIndex (familyId);
	DEBUG_FATAL (familyIndex == -1, ("family with id %i not found", familyId));

	return familyList [familyIndex]->getSurfaceProperties ();
}

//-------------------------------------------------------------------

void ShaderGroup::loadSurfaceProperties ()
{
	int i;
	for (i = 0; i < familyList.getNumberOfElements (); ++i)
		familyList [i]->loadSurfaceProperties ();
}

//-------------------------------------------------------------------

void ShaderGroup::Family::loadSurfaceProperties ()
{
	if (surfaceProperties)
		const_cast<ObjectTemplate*> (surfaceProperties)->releaseReference ();

	surfaceProperties = 0;

	if (surfacePropertiesName && *surfacePropertiesName)
	{
		if (TreeFile::exists (surfacePropertiesName))
			surfaceProperties = ObjectTemplateList::fetch (surfacePropertiesName);
		else
			DEBUG_WARNING (true, ("Shader family %s could not open properties file %s\n", name, surfacePropertiesName));
	}
}

//-------------------------------------------------------------------


