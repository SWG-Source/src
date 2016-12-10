//===================================================================
//
// FractalGroup.h
// asommers 9-17-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_FractalGroup_H
#define INCLUDED_FractalGroup_H

//===================================================================

class Iff;
class MultiFractal;

//===================================================================

class FractalGroup
{
public:

	FractalGroup ();
	~FractalGroup ();

	//-- creation routines (for editor)
	void                  load (Iff& iff);
	void                  save (Iff& iff) const;
	void                  reset ();

	void                  prepare (int cacheX, int cacheY);

	//--
	const char*           getFamilyName (int familyId) const;
	void                  setFamilyName (int familyId, const char* name);
	const MultiFractal*   getFamilyMultiFractal (int familyId) const;
	MultiFractal*         getFamilyMultiFractal (int familyId);

	//-- family id routines
	void                  addFamily (int familyId, const char* name);
	void                  removeFamily (int familyId);
	bool                  hasFamily (int familyId) const;
	bool                  findFamily (const char* name, int& familyId) const;

	//-- family index routines
	int                   getNumberOfFamilies () const;
	int                   getFamilyId (int familyIndex) const;
	void                  setFamilyId (int familyIndex, int newId);

	//-- used to convert from the old terrain format to the new terrain format
	int                   createFamily (const MultiFractal* multiFractal, const char* baseName);

private:

	//-- list of families
	class Family;
	typedef std::map<int, Family*> FamilyMap;
	FamilyMap*            m_familyMap;

private:

	const Family*         getFamily (int familyId) const;
	Family*               getFamily (int familyId);
	const Family*         getFamily (const char* familyName) const;
	Family*               getFamily (const char* familyName);
	
	int                   createUniqueFamilyId () const;
	std::string           createUniqueFamilyName (const char* baseName) const;

	void                  load_0000 (Iff& iff);

private:

	FractalGroup (const FractalGroup& rhs);
	FractalGroup& operator= (const FractalGroup& rhs);
};

//===================================================================

#endif
