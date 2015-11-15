// ======================================================================
//
// SetupSharedGame.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupSharedGame_H
#define INCLUDED_SetupSharedGame_H

class StringId;

// ======================================================================

class SetupSharedGame
{
public:

	typedef void (*DebugBadStringsFunc)    (const StringId &, bool);

	class Data
	{
	public:

		Data ();

		void  setUseGameScheduler (bool useIt);
		bool  getUseGameScheduler () const;

		void  setUseMountValidScaleRangeTable (bool useIt);
		bool  getUseMountValidScaleRangeTable () const;

		void  setUseWearableAppearanceMap(bool useIt);
		bool  getUseWearableAppearanceMap() const;

		void  setUseClientCombatManagerSupport(bool useIt);
		bool  getUseClientCombatManagerSupport() const;

		DebugBadStringsFunc m_debugBadStringsFunc;

	private:

		bool  m_useGameScheduler;
		bool  m_useMountValidScaleRangeTable;
		bool  m_useWearableAppearanceMap;
		bool  m_useClientCombatManagerSupport;
	};

public:

	static void install (const Data& data);

private:

	SetupSharedGame ();
	SetupSharedGame (const SetupSharedGame&);
	SetupSharedGame& operator= (const SetupSharedGame&);
};

// ======================================================================

#endif
