//===================================================================
//
// EnvironmentGroup.h
// asommers 9-17-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_EnvironmentGroup_H
#define INCLUDED_EnvironmentGroup_H

//===================================================================

#include "sharedFoundation/ArrayList.h"
#include "sharedMath/PackedRgb.h"

class ColorRamp256;
class Iff;

//===================================================================

class EnvironmentGroup
{
public:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// Info holds info about an environment
	//
	struct Info
	{
	public:

		Info();
		~Info();

		int getFamilyId() const;
		void setFamilyId(int familyId);

	private:

		uint8 m_familyId;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	class Family;

private:

	//-- list of families
	ArrayList<Family*> familyList;

	//--
	PackedRgb errorFamilyColor;

private:

	int                   findFamilyIndex (int familyId) const;
	int                   findFamilyIndex (const PackedRgb& desiredColor) const;

private:

	void                  load_0000 (Iff& iff);
	void                  load_0001 (Iff& iff);
	void                  load_0002 (Iff& iff);

private:

	EnvironmentGroup (const EnvironmentGroup& rhs);
	EnvironmentGroup& operator= (const EnvironmentGroup& rhs);

public:

	EnvironmentGroup ();
	~EnvironmentGroup ();

	const Info            getDefaultEnvironment () const;
	const Info            chooseEnvironment (int familyId) const;
	const Info            chooseEnvironment (const PackedRgb& desiredColor) const;

	//-- creation routines (for editor)
	void                  load (Iff& iff);
	void                  save (Iff& iff) const;
	void                  removeAllFamilies ();

	//-- family id routines
	void                  addFamily (int familyId, const char* name, const PackedRgb& color);
	void                  removeFamily (int familyId);
	bool                  hasFamily (int familyId) const;
	bool                  hasFamily (const char* name) const;

	int                   getFamilyId (const PackedRgb& desiredColor) const;
	int                   getFamilyId (const char* name) const;
	const char*           getFamilyName (int familyId) const;
	void                  setFamilyName (int familyId, const char* name);
	const PackedRgb&      getFamilyColor (int familyId) const;
	void                  setFamilyColor (int familyId, const PackedRgb& prgb);
	float                 getFamilyFeatherClamp (int familyId) const;
	void                  setFamilyFeatherClamp (int familyId, float featherClamp);

	void                  copyFamily (int destinationFamilyId, int sourceFamilyId);

	//-- family index routines
	int                   getNumberOfFamilies () const;
	int                   getFamilyId (int familyIndex) const;
	void                  setFamilyId (int familyIndex, int newId);
};

//===================================================================

#endif
