//===================================================================
//
// SetupSharedTerrain.h
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_SetupSharedTerrain_H
#define INCLUDED_SetupSharedTerrain_H

//===================================================================

class SetupSharedTerrain
{
public:

	class Data
	{
	public:

		Data ();

	private:

		bool m_allowInactiveLayerItems;

	private:

		friend class SetupSharedTerrain;
	};

public:

	static void install (const Data& data);
	static void remove ();

	static bool isInstalled ();

	static void setupGameData (Data& data);
	static void setupToolData (Data& data);

private:

	SetupSharedTerrain ();
	SetupSharedTerrain (const SetupSharedTerrain&);
	SetupSharedTerrain& operator= (const SetupSharedTerrain&);
};

//===================================================================

#endif
