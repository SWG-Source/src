//===================================================================
//
// SetupSharedUtility.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SetupSharedUtility_H
#define INCLUDED_SetupSharedUtility_H

//===================================================================

class SetupSharedUtility
{
public:

	class Data
	{
		friend class SetupSharedUtility;

	public:

		Data ();

	public:

		bool m_allowFileCaching;
	};

public:

	static void install (const Data& data);
	static void remove ();

	static void setupGameData (Data& data);
	static void setupToolData (Data& data);
	static void installFileManifestEntries ();

private:

	SetupSharedUtility ();
	SetupSharedUtility (const SetupSharedUtility&);
	SetupSharedUtility& operator= (const SetupSharedUtility&);
};

//===================================================================

#endif
