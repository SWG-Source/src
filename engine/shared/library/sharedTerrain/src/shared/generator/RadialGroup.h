//===================================================================
//
// RadialGroup.h
// asommers 9-17-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_RadialGroup_H
#define INCLUDED_RadialGroup_H

//===================================================================

#include "sharedFoundation/ArrayList.h"
#include "sharedMath/PackedRgb.h"

class Iff;

//===================================================================

class RadialGroup
{
public:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// Info holds info about a radial group and its child choice
	//
	struct Info
	{
	public:

		Info();
		~Info();

		int getFamilyId() const;
		void setFamilyId(int familyId);
		float getChildChoice() const;
		void setChildChoice(float childChoice);

	private:

		uint8 m_familyId;
		uint8 m_childChoice;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// FamilyChildData allows creation of RadialGroups
	//
	struct FamilyChildData
	{
	public:

		int         familyId;
		float       weight;
		const char* shaderTemplateName;
		float       distance;
		float       minWidth;
		float       maxWidth;
		float       minHeight;
		float       maxHeight;
		bool        maintainAspectRatio;
		float       period;
		float       displacement;
		bool        shouldSway;
		bool        alignToTerrain;
		bool        createPlus;

	public:
		
		FamilyChildData ();
		~FamilyChildData ();
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	class Family;

private:

	ArrayList<Family *> familyList;
	PackedRgb errorFamilyColor;

private:

	int                   findFamilyIndex (int familyId) const;
	int                   findFamilyIndex (const PackedRgb& desiredColor) const;

private:

	void                  load_0000 (Iff& iff);
	void                  load_0001 (Iff& iff);
	void                  load_0002 (Iff& iff);
	void                  load_0003 (Iff& iff);
	void                  load_0004 (Iff& iff);

private:

	RadialGroup (const RadialGroup& rhs);
	RadialGroup& operator= (const RadialGroup& rhs);

public:

	RadialGroup ();
	~RadialGroup ();

	const Info            getDefaultRadial () const;
	const Info            chooseRadial (int familyId) const;
	const Info            chooseRadial (const PackedRgb& desiredColor) const;

	FamilyChildData const & createRadial(RadialGroup::Info const & rfgi) const;

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
	float                 getFamilyDensity (int familyId) const;
	void                  setFamilyDensity (int familyId, float density);

	//-- family index routines
	int                   getNumberOfFamilies () const;
	int                   getNumberOfChildren (int familyIndex) const;
	FamilyChildData const & getChild(int familyIndex, int childIndex) const;
	int                   getFamilyId (int familyIndex) const;
	void                  setFamilyId (int familyIndex, int newId);
};

//===================================================================

#endif
