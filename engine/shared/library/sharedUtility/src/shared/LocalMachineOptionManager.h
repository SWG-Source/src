// ======================================================================
//
// LocalMachineOptionManager.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_LocalMachineOptionManager_H
#define INCLUDED_LocalMachineOptionManager_H

// ======================================================================

class LocalMachineOptionManager
{
public:

	static void install ();
	static void remove ();

	static void save ();

	static void registerOption (bool & variable, char const * section, char const * name, const int version = 0);
	static void registerOption (float & variable, char const * section, char const * name, const int version = 0);
	static void registerOption (int & variable, char const * section, char const * name, const int version = 0);
	static void registerOption (std::string & variable, char const * section, char const * name, const int version = 0);
	static void registerOption (Unicode::String & variable, char const * section, char const * name, const int version = 0);

	static float findFloat(char const * section, char const * name, float defaultValue);

private:

	LocalMachineOptionManager ();
	~LocalMachineOptionManager ();
	LocalMachineOptionManager (LocalMachineOptionManager const &);
	LocalMachineOptionManager & operator= (LocalMachineOptionManager const &);
};

// ======================================================================

#endif
