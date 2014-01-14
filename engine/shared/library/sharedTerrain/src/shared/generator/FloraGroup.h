//===================================================================
//
// FloraGroup.h
// asommers 9-17-2000
//
// copyright 2000, verant interactive
//
//--
//
// A FloraGroup contains lists of families of meshes organized by id. 
// A flora family is a list of valid appearances for a specified id. The flora
// family also stores the name of the flora, and its appearanceTemplate.
// The flora group will return created appearances for use with the terrain
// system. In the event that requested flora family id does not
// exist, the flora group will return a [clearly visible] default flora.
// Families will only create flora when they are chosen.
//
//===================================================================

#ifndef FLORAGROUP_H
#define FLORAGROUP_H

//===================================================================

#include "sharedFoundation/ArrayList.h"
#include "sharedMath/PackedRgb.h"

class Iff;
class AppearanceTemplate;

//===================================================================

class FloraGroup
{
public:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// Info holds info about flora for the flora map
	//
	struct Info
	{
	public:

		Info ();
		~Info ();

		int   getFamilyId () const;
		void  setFamilyId (int familyId);
		float getChildChoice () const;
		void  setChildChoice (float childChoice);

	private:

		uint8 m_familyId;
		uint8 m_childChoice;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// FamilyChildData allows creation of FloraGroups
	//
	struct FamilyChildData
	{
	public:

		int         familyId;
		float        weight;
		const char* appearanceTemplateName;
		bool        shouldSway;
		float        period;
		float        displacement;
		bool        alignToTerrain;
		bool        shouldScale;
		float        minimumScale;
		float        maximumScale;

	public:

		FamilyChildData ();
		~FamilyChildData ();
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	class Family;

private:

	//-- list of families
	ArrayList<Family*>    familyList;

	//--
	PackedRgb             errorFamilyColor;

private:

	int                   findFamilyIndex (int familyId) const;
	int                   findFamilyIndex (const PackedRgb& desiredColor) const;

	void                  load_0001 (Iff& iff);
	void                  load_0002 (Iff& iff);
	void                  load_0003 (Iff& iff);
	void                  load_0004 (Iff& iff);
	void                  load_0005 (Iff& iff);
	void                  load_0006 (Iff& iff);
	void                  load_0007 (Iff& iff);
	void                  load_0008 (Iff& iff);

private:

	FloraGroup (const FloraGroup& rhs);
	FloraGroup& operator= (const FloraGroup& rhs);

public:

	FloraGroup ();
	~FloraGroup ();

	const Info            getDefaultFlora () const;
	const Info            chooseFlora (int familyId) const;
	const Info            chooseFlora (const PackedRgb& desiredColor) const;

	FamilyChildData const & createFlora(FloraGroup::Info const & rfgi) const;

	//-- creation routines (for editor)
	void                  load (Iff& iff);
	void                  save (Iff& iff) const;
	void                  removeAllFamilies ();

	//-- family id routines
	void                  addFamily (int familyId, const char* name, const PackedRgb& color);
	void                  removeFamily (int familyId);
	void                  addChild (const FamilyChildData& familyChildData);
	void                  removeChild (const FamilyChildData& familyChildData);
	bool                  hasFamily (int familyId) const;
	bool                  hasFamily (const char* name) const;

	int                   getFamilyId (const PackedRgb& desiredColor) const;
	int                   getFamilyId (const char* name) const;
	const char*           getFamilyName (int familyId) const;
	void                  setFamilyName (int familyId, const char* name);
	const PackedRgb&      getFamilyColor (int familyId) const;
	void                  setFamilyColor (int familyId, const PackedRgb& prgb);
	int                   getFamilyNumberOfChildren (int familyId) const;
	void                  renameChild (int familyId, const char* oldName, const char* newName);
	FamilyChildData const & getFamilyChild(int familyId, char const * childName) const;
	FamilyChildData const & getFamilyChild(int familyId, int childIndex) const;
	void                  setFamilyChild (int familyId, const FamilyChildData& familyChildData);
	float                  getFamilyDensity (int familyId) const;
	void                  setFamilyDensity (int familyId, float density);
	bool                  getFamilyFloats (int familyId) const;
	void                  setFamilyFloats (int familyId, bool floats);

	//-- family index routines
	int                   getNumberOfFamilies () const;
	int                   getNumberOfChildren (int familyIndex) const;
	FamilyChildData const & getChild(int familyIndex, int childIndex) const;
	int                   getFamilyId (int familyIndex) const;
	void                  setFamilyId (int familyIndex, int newId);
};

//===================================================================

#endif
