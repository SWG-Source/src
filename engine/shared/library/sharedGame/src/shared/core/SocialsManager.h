//======================================================================
//
// SocialsManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SocialsManager_H
#define INCLUDED_SocialsManager_H

//======================================================================

class SocialsManager
{
public:

	typedef std::map<std::string, uint32> StringIntMap;

	static void                         install ();
	static void                         remove  ();

	static uint32                       getSocialTypeByName (const std::string & name);
	static bool                         getSocialNameByType (const uint32, std::string & name);
	static bool                         getSocialNameByCrc  (const uint32, std::string & name);

	static void                         getCanonicalSocials (StringIntMap & sim);

private:

	static void                         load                (const std::string & filename);
};

//======================================================================

#endif
