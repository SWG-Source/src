// ======================================================================
//
// DebugKey.h
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DebugKey_H
#define INCLUDED_DebugKey_H

// ======================================================================

class DebugKey
{
	friend class Os;

public:

	static DLLEXPORT void registerFlag(bool &variable, const char *name);

	static void newFrame();
	static void lostFocus();

	static bool isActive();
	static void setCurrentFlag(const bool *flag);

	static void pressKey(int i);
	static void releaseKey(int i);

	static DLLEXPORT bool isPressed(int i);
	static DLLEXPORT bool isDown(int i);

private:

	struct Flag
	{
		bool           *variable;
		const char     *name;
		bool operator <(const Flag &rhs) const;
	};

	typedef std::vector<Flag> FlagVector;

private:

	static bool        *ms_currentFlag;
	static bool         ms_pressed[10];
	static bool         ms_down[10];
	static FlagVector   ms_flags;
};

// ======================================================================

#endif
