//======================================================================
//
// MoodManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MoodManager_H
#define INCLUDED_MoodManager_H

//======================================================================

class LocalizedStringTable;

//----------------------------------------------------------------------

class MoodManager
{
public:

	typedef std::map<std::string, uint32> StringIntMap;

	static void                          install ();
	static void                          remove ();

	static uint32                        getMoodByCanonicalName (const std::string & canonicalName);
	static bool                          getCanonicalName       (uint32 id, std::string & name);

	static void                          getCanonicalMoods      (StringIntMap & sim);

private:
	static void                          load (const std::string & filename);
};

//======================================================================

#endif
