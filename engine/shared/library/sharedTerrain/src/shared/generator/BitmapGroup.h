//===================================================================
//
// BitmapGroup.h
//
// copyright 2004, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_BitmapGroup_H
#define INCLUDED_BitmapGroup_H

//===================================================================

class Iff;
class Image;
//===================================================================

class BitmapGroup
{
public:

	BitmapGroup ();
	~BitmapGroup ();

	//-- creation routines (for editor)
	void                  load (Iff& iff);
	void                  save (Iff& iff) const;
	void                  reset ();

	void                  prepare (int cacheX, int cacheY);

	//--
	const char*           getFamilyName (int familyId) const;
	void                  setFamilyName (int familyId, const char* name);
	void                  loadFamilyBitmap(int familyId, const char* name);
	const Image*   getFamilyBitmap (int familyId) const;
	Image*         getFamilyBitmap (int familyId);
	void                  reloadAllFamilyBitmaps(); // reloads all the bitmaps - called from the terrain editor

	//-- family id routines
	void                  addFamily (int familyId, const char* name, const char* bitmapName);
	void                  removeFamily (int familyId);
	bool                  hasFamily (int familyId) const;
	bool                  findFamily (const char* name, int& familyId) const;

	//-- family index routines
	int                   getNumberOfFamilies () const;
	int                   getFamilyId (int familyIndex) const;
	void                  setFamilyId (int familyIndex, int newId);

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

	BitmapGroup (const BitmapGroup& rhs);
	BitmapGroup& operator= (const BitmapGroup& rhs);
};

//===================================================================

#endif
