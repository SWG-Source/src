// ======================================================================
//
// CurrentUserOptionManager.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_CurrentUserOptionManager_H
#define INCLUDED_CurrentUserOptionManager_H

// ======================================================================

class CurrentUserOptionManager
{
public:

	static void install ();
	static void remove ();

	static void load (char const * userName);
	static void save ();

	static void registerOption (bool & variable, char const * section, char const * name, const int version = 0);
	static void registerOption (float & variable, char const * section, char const * name, const int version = 0);
	static void registerOption (int & variable, char const * section, char const * name, const int version = 0);
	static void registerOption (std::string & variable, char const * section, char const * name, const int version = 0);
	static void registerOption (Unicode::String & variable, char const * section, char const * name, const int version = 0);

private:

	CurrentUserOptionManager ();
	~CurrentUserOptionManager ();
	CurrentUserOptionManager (CurrentUserOptionManager const &);
	CurrentUserOptionManager & operator= (CurrentUserOptionManager const &);
};

// ======================================================================

#endif
